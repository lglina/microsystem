#ifndef AGAPE_TIMERS_HIGH_RES_H
#define AGAPE_TIMERS_HIGH_RES_H

#include "Timers/Timer.h"

#include <chrono>

using namespace std::chrono;

namespace Agape
{

namespace Timers
{

class HighRes : public Timer
{
public:
    HighRes();
    virtual long ms();
    virtual long us();
    virtual void reset();

private:
    time_point<high_resolution_clock> m_startTime;
};

} // namespace Timers

} // namespace Agape

#endif // AGAPE_TIMERS_HIGH_RES_H
