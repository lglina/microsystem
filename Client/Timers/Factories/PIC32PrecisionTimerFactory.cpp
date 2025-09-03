#include "Timers/PIC32PrecisionTimer.h"
#include "PIC32PrecisionTimerFactory.h"

namespace Agape
{

namespace Timers
{

namespace Factories
{

Timer* PIC32PrecisionTimerFactory::makeTimer()
{
    return new Timers::PIC32Precision;
}

} // namespace Factories

} // namespace Timers

} // namespace Agape
