#ifndef AGAPE_TIMER_FACTORY_H
#define AGAPE_TIMER_FACTORY_H

#include "Timers/Timer.h"

namespace Agape
{

namespace Timers
{

class Factory
{
public:
    virtual ~Factory() {};

    virtual Timer* makeTimer() = 0;
};

} // namespace Timers

} // namespace Agape

#endif // AGAPE_TIMER_FACTORY_H
