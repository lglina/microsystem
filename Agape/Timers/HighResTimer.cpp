#include "HighResTimer.h"

#include <chrono>

using namespace std::chrono;

namespace Agape
{

namespace Timers
{

HighRes::HighRes()
{
    reset();
}

long HighRes::ms()
{
    return us() / 1000;
}

long HighRes::us()
{
    auto endTime( high_resolution_clock::now() );
    auto duration( duration_cast<microseconds>( endTime - m_startTime ) );
    return duration.count();
}

void HighRes::reset()
{
    m_startTime = high_resolution_clock::now();
}

} // namespace Timers

} // namespace Agape

