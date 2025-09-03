#ifndef AGAPE_TIMERS_PIC32_PRECISION_H
#define AGAPE_TIMERS_PIC32_PRECISION_H

#include "Timers/Timer.h"

namespace Agape
{

namespace Timers
{

class PIC32Precision : public Timer
{
public:
    PIC32Precision();

    virtual long ms();
    virtual long us();
    virtual void reset();

    virtual void usleep( long us );
};

} // namespace Timers

} // namespace Agape

#endif // AGAPE_TIMERS_PIC32_PRECISION_H
