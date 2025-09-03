#include "CTimer.h"

#include <sys/time.h>
#include <stddef.h>
#include <unistd.h>

namespace Agape
{

namespace Timers
{

C::C() :
  m_initialMs( currentMs() )
{
}

long C::ms()
{
    return( currentMs() - m_initialMs );
}

void C::reset()
{
    m_initialMs = currentMs();
}

void C::usleep( long us )
{
    ::usleep( us );
}

unsigned long long C::currentMs()
{
    struct timeval tv;
    ::gettimeofday( &tv, NULL );
    return( ( (unsigned long long)( tv.tv_sec ) * 1000 ) +
            ( (unsigned long long)( tv.tv_usec ) / 1000 ) );
}

} // namespace Timers

} // namespace Agape
