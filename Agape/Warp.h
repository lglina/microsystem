#ifndef AGAPE_WARP_H
#define AGAPE_WARP_H

#include "String.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class Timer;

class Warp
{
public:
    Warp( const String& taskName, bool doEngage = true );
    ~Warp();

    void engage();
    void report();

    static void setTimerFactory( Timers::Factory* timerFactory );

private:
    String m_taskName;
    Timer* m_timer;

    static Timers::Factory* s_timerFactory;

    bool m_engaged;
};

} // namespace Agape

#endif // AGAPE_WARP_H
