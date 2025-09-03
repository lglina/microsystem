#include "Timers/CTimer.h"
#include "CTimerFactory.h"

namespace Agape
{

namespace Timers
{

namespace Factories
{

Timer* C::makeTimer()
{
    return new Timers::C;
}

} // namespace Factories

} // namespace Timers

} // namespace Agape
