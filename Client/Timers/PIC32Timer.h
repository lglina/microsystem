#ifndef AGAPE_TIMERS_PIC32_H
#define AGAPE_TIMERS_PIC32_H

#include "InterruptHandler.h"
#include "Timers/Timer.h"

namespace Agape
{

namespace Timers
{

class PIC32Absolute;

class PIC32Timer : public Timer
{
public:
    PIC32Timer();

    virtual long ms();
    virtual long us();
    virtual void reset();

    virtual void usleep( long us );

private:
    long long m_startTime;

    PIC32Absolute* m_absoluteTimer;
};

} // namespace Timers

} // namespace Agape

#endif // AGAPE_TIMERS_PIC32_H
