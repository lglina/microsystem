#ifndef AGAPE_TIMERS_C_H
#define AGAPE_TIMERS_C_H

#include "Timer.h"

namespace Agape
{

namespace Timers
{

class C : public Timer
{
public:
    C();

    virtual long ms();
    virtual void reset();

    virtual void usleep( long us );

private:
    unsigned long long currentMs();

    unsigned long long m_initialMs;
};

} // namespace Timers

} // namespace Agape

#endif // AGAPE_TIMERS_C_H
