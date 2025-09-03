#ifndef AGAPE_TIMER_FACTORIES_PIC32_PRECISION_H
#define AGAPE_TIMER_FACTORIES_PIC32_PRECISION_H

#include "Timers/Factories/TimerFactory.h"

namespace Agape
{

namespace Timers
{

namespace Factories
{

class PIC32PrecisionTimerFactory : public Factory
{
public:
    virtual Timer* makeTimer();
};

} // namespace Factories

} // namespace Timers

} // namespace Agape

#endif // AGAPE_TIMER_FACTORIES_PIC32_PRECISION_H
