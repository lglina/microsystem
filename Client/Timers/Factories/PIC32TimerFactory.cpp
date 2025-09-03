#include "Timers/PIC32Timer.h"
#include "PIC32TimerFactory.h"

namespace Agape
{

namespace Timers
{

namespace Factories
{

Timer* PIC32TimerFactory::makeTimer()
{
    return new Timers::PIC32Timer;
}

} // namespace Factories

} // namespace Timers

} // namespace Agape
