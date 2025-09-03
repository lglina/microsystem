#include "CEventClock.h"
#include "StringConstants.h"
#include "TupleRouter.h"
#include "Tuple.h"

#include <chrono>

namespace Agape
{

using namespace Linda2;

using namespace std::chrono;

namespace EventClocks
{

C::C( TupleRouter& tupleRouter ) :
  EventClock( tupleRouter ),
  m_lastTime( system_clock::now() )
{
}

void C::run()
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
    }
}

} // namespace EventClocks

} // namespace Agape
