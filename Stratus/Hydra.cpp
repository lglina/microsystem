#include "Hydra.h"

#include "Loggers/Logger.h"
#include "Timers/Factories/CTimerFactory.h"
#include "TupleRoutes/TupleRoute.h"
#include "TupleRouter.h"

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

using namespace Agape::Linda2;

namespace Agape
{

namespace Stratus
{

Hydra::Hydra() :
  m_tupleRouter( m_tupleDispatcher, "Hydra", m_timerFactory ),
  m_stopping( false )
{
    m_routingThread.reset( new std::thread( std::bind( &Hydra::route, this ) ) );
}

Hydra::~Hydra()
{
    {
    std::scoped_lock lock( m_mutex );
    m_stopping = true;
    m_incomingTuple.notify_all();
    }

    m_routingThread->join();
}

void Hydra::addRoute( TupleRoute* route )
{
    LOG_DEBUG( "Hydra: Adding route" );
    std::scoped_lock lock( m_mutex );
    m_tupleRouter.addRoute( route, false );
}

void Hydra::removeRoute( TupleRoute* route )
{
    LOG_DEBUG( "Hydra: Removing route" );
    std::scoped_lock lock( m_mutex );
    m_tupleRouter.removeRoute( route );
}

void Hydra::signalIncoming()
{
#ifdef LOG_STRATUS
    LOG_DEBUG( "Hydra: Signalling incoming" );
#endif
    std::scoped_lock lock( m_mutex );
    m_incomingTuple.notify_all();
}

void Hydra::route()
{
    LOG_DEBUG( "Hydra: Routing thread started" );
    while( !m_stopping )
    {
#ifdef LOG_STRATUS
        LOG_DEBUG( "Hydra: Waiting for incoming" );
#endif
        std::unique_lock lock( m_mutex );
        m_incomingTuple.wait( lock );

#ifdef LOG_STRATUS
        LOG_DEBUG( "Hydra: Handling incoming" );
#endif
        m_tupleRouter.run();
    }
}

} // namespace Stratus

} // namespace Agape
