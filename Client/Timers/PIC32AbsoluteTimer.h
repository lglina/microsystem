#ifndef AGAPE_TIMERS_PIC32_ABSOLUTE_H
#define AGAPE_TIMERS_PIC32_ABSOLUTE_H

#include "InterruptHandler.h"
#include "Timers/Timer.h"

namespace Agape
{

class BusController;

namespace Timers
{

class PIC32Absolute : public InterruptHandler
{
public:
    PIC32Absolute( BusController* bus );
    ~PIC32Absolute();

    virtual long long us();

    void start();
    void stop();

    virtual void handleInterrupt( enum InterruptDispatcher::InterruptVector vector );

    static PIC32Absolute* getInstance();

private:
    BusController* m_bus;

    volatile long long m_us;

    static PIC32Absolute* s_instance;
};

} // namespace Timers

} // namespace Agape

#endif // AGAPE_TIMERS_PIC32_ABSOLUTE_H
