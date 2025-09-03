#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "EventTimerActor.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"

namespace
{
    static int tickPeriod( 100 );
    static int clockPeriod( 1000 );
} // Anonymous namespace

namespace Agape
{

namespace Linda2
{

namespace Actors
{

namespace NativeActors
{

EventTimer::EventTimer( Timers::Factory& timerFactory, TupleRouter& tupleRouter ) :
  Native( _Timer ),
  m_tupleRouter( tupleRouter ),
  m_timer( timerFactory.makeTimer() ),
  m_lastS( 0 ),
  m_lastMs( 0 )
{
    m_tupleRouter.registerActor( this );
}

EventTimer::~EventTimer()
{
    m_tupleRouter.deregisterActor( this );
    delete( m_timer );
}

bool EventTimer::accept( Tuple& tuple )
{
    if( ( m_tupleRouter.sourceActor( tuple ) == _Clock ) &&
        ( m_tupleRouter.tupleType( tuple ) == _Time ) )
    {
        m_timer->reset();
        m_lastS = tuple[_now];
        m_lastMs = 0;
        sendTuple();

        return true;
    }

    return false;
}

void EventTimer::run()
{
    long currentMs( m_timer->ms() );

    if( ( m_lastS != 0 ) &&
        ( ( currentMs - m_lastMs ) >= tickPeriod ) &&
        ( ( currentMs - m_lastMs ) <= ( clockPeriod - ( tickPeriod / 2 ) ) ) )
    {
        m_lastMs += tickPeriod;
        sendTuple();
    }
}

void EventTimer::sendTuple()
{
    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _Timer );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _Tick );
    tuple[_now] = ( m_lastS * 1000 ) + m_lastMs;

    m_tupleRouter.route( tuple );
}

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape
