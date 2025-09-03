#include "Timers/HighResTimer.h"
#include "HighResTimerFactory.h"

namespace Agape
{

namespace Timers
{

namespace Factories
{

Timer* HighRes::makeTimer()
{
    return new Timers::HighRes;
}

} // namespace Factories

} // namespace Timers

} // namespace Agape
