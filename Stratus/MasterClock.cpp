#include "TupleRoutes/QueueingTupleRoute.h"
#include "MasterClock.h"
#include "Hydra.h"
#include "StringConstants.h"
#include "String.h"
#include "TupleRouter.h"
#include "Tuple.h"

#include <chrono>

using namespace std::chrono;
using namespace std::literals;

namespace Agape
{

namespace Stratus
{

MasterClock::MasterClock( Hydra& hydra ) :
  m_hydra( hydra ),
  m_tupleRouter( m_tupleDispatcher,
                 "ClockRouter",
                 m_timerFactory ),
  m_hydraNearTupleRoute( "ClockNear" ),
  m_hydraFarTupleRoute( "ClockFar" ),
  m_lastTime( system_clock::now() )
{
    m_hydraNearTupleRoute.setPartner( &m_hydraFarTupleRoute );
    m_hydraFarTupleRoute.setPartner( &m_hydraNearTupleRoute );

    m_hydra.addRoute( &m_hydraFarTupleRoute );

    m_tupleRouter.setMyID( "Stratus" );

    m_tupleRouter.addRoute( &m_hydraNearTupleRoute, true ); // Default route.
}

MasterClock::~MasterClock()
{
    m_hydra.removeRoute( &m_hydraFarTupleRoute );
}

void MasterClock::run()
{
    if( ( system_clock::now() - m_lastTime ) >= 1s )
    {
        m_lastTime += 1s;

        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _Clock );
        TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
        TupleRouter::setTupleType( tuple, _Time );
        tuple[_now] = (double)duration_cast< seconds >( system_clock::now().time_since_epoch() ).count();

        m_tupleRouter.route( tuple );
        m_tupleRouter.run();
        m_hydra.signalIncoming();
    }
}

} // namespace Stratus

} // namespace Agape
