#include "NullTimer.h"

namespace Agape
{

namespace Timers
{

Null::~Null()
{
}

long Null::ms()
{
    return 0;
}

void Null::reset()
{
}

} // namespace Timers

} // namespace Agape
