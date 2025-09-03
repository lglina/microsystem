#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Promise.h"
#include "TupleRouter.h"
#include "Value.h"

namespace
{
    long pollIntervalms( 10 );
    long timeoutms( 10000 );
} // Anonymous namespace

namespace Agape
{

namespace Linda2
{

Promise::Future::Future( Promise& promise ) :
  m_promise( promise )
{
}

bool Promise::Future::get( Value& value )
{
    if( m_promise.m_tupleRouter && m_promise.m_timerFactory && !m_promise.m_set )
    {
        Timer* timer( m_promise.m_timerFactory->makeTimer() );
        while( !m_promise.m_set && ( timer->ms() < timeoutms ) )
        {
            m_promise.m_tupleRouter->run(); // May call accept() on caller who will set() a response.

            if( !m_promise.m_set )
            {
                timer->usleep( pollIntervalms * 1000 );
            }
        }

        if( !m_promise.m_set )
        {
            LOG_DEBUG( "Future timed out" );
        }

        delete( timer );
    }

    value = m_promise.m_value;

    return m_promise.m_success;
}

bool Promise::Future::get()
{
    Value value;
    return( get( value ) );
}

Promise::Promise() :
  m_tupleRouter( nullptr ),
  m_timerFactory( nullptr ),
  m_set( false ),
  m_success( false )
{
}

Promise::Promise( TupleRouter* tupleRouter, Timers::Factory* timerFactory ) :
  m_tupleRouter( tupleRouter ),
  m_timerFactory( timerFactory ),
  m_set( false ),
  m_success( false )
{
}

Promise::Future Promise::getFuture()
{
    return Future( *this );
}

bool Promise::set( const Value& successValue, const Value& returnValue )
{
    m_value = returnValue;
    m_set = true;
    return( m_success = ( (int)successValue == 1 ) );
}

bool Promise::set( const Value& successValue )
{
    m_set = true;
    return( m_success = ( (int)successValue == 1 ) );
}

void Promise::set()
{
    m_set = true;
    m_success = true;
}

} // namespace Linda2

} // namespace Agape
