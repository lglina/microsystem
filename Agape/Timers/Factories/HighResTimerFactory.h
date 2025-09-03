#ifndef AGAPE_TIMER_FACTORIES_HIGH_RES_H
#define AGAPE_TIMER_FACTORIES_HIGH_RES_H

#include "Timers/Factories/TimerFactory.h"

namespace Agape
{

namespace Timers
{

namespace Factories
{

class HighRes : public Factory
{
public:
    virtual Timer* makeTimer();
};

} // namespace Factories

} // namespace Timers

} // namespace Agape

#endif // AGAPE_TIMER_FACTORIES_HIGH_RES_H
