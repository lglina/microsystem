#ifndef AGAPE_TIMER_FACTORIES_PIC32_H
#define AGAPE_TIMER_FACTORIES_PIC32_H

#include "Timers/Factories/TimerFactory.h"

namespace Agape
{

namespace Timers
{

namespace Factories
{

class PIC32TimerFactory : public Factory
{
public:
    virtual Timer* makeTimer();
};

} // namespace Factories

} // namespace Timers

} // namespace Agape

#endif // AGAPE_TIMER_FACTORIES_PIC32_H
