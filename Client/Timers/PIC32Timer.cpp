#include "InterruptHandler.h"
#include "PIC32AbsoluteTimer.h"
#include "PIC32Timer.h"

#include <math.h>

namespace Agape
{

namespace Timers
{

PIC32Timer::PIC32Timer() :
  m_startTime( 0 ),
  m_absoluteTimer( nullptr )
{
    m_absoluteTimer = PIC32Absolute::getInstance();
    reset();
}

long PIC32Timer::ms()
{
    return( ::lround( us() / 1000 ) );
}

long PIC32Timer::us()
{
    if( m_absoluteTimer )
    {
        return( m_absoluteTimer->us() - m_startTime );
    }
    
    return 0;
}

void PIC32Timer::reset()
{
    if( m_absoluteTimer )
    {
        m_startTime = m_absoluteTimer->us();
    }
}

void PIC32Timer::usleep( long _us )
{
    long long prevStartTime( m_startTime );
    reset();
    while( us() < _us ) {}
    m_startTime = prevStartTime;
}

} // namespace Timers

} // namespace Agape
