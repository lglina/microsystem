#ifndef AGAPE_TIMERS_NULL_H
#define AGAPE_TIMERS_NULL_H

#include "Timer.h"

namespace Agape
{

namespace Timers
{

class Null : public Timer
{
public:
    virtual ~Null();
    
    virtual long ms();
    virtual void reset();
};

} // namespace Timers

} // namespace Agape

#endif // AGAPE_TIMERS_NULL_H
