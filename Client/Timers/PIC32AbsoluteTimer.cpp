#if defined(__PIC32MX__)
#include "BusAddresses.h"
#include "BusController.h"
#endif

#include "InterruptHandler.h"
#include "PIC32AbsoluteTimer.h"

#include "cpu.h"

#include <xc.h>

namespace
{
    const int timerInterval( 1000 ); // us
    const unsigned long timerReload( PBCLK_FREQ / 1000000ul * (unsigned long)timerInterval );
} // Anonymous namespace

namespace Agape
{

namespace Timers
{

PIC32Absolute* PIC32Absolute::s_instance;

PIC32Absolute::PIC32Absolute( BusController* bus ) :
  m_bus( bus ),
  m_us( 0 )
{
    s_instance = this;
    Agape::InterruptDispatcher::instance()->registerHandler( Agape::InterruptDispatcher::timer2, this );
    start();
}

PIC32Absolute::~PIC32Absolute()
{
    stop();
    Agape::InterruptDispatcher::instance()->deregisterHandler( Agape::InterruptDispatcher::timer2 );
    s_instance = nullptr;
}

long long PIC32Absolute::us()
{
    // TODO: We could improve this by tying TMR2 and TMR3 together for a 32-bit
    // timer, then interrupting only on the second and just reading the
    // fractional part directly off the TMR2 register...
    // Probably not an issue, though, if we're happy for a low-ish resolution.
    IEC0CLR = _IEC0_T2IE_MASK;
    long long ret( m_us );
    IEC0SET = _IEC0_T2IE_MASK;

    return ret;
}

void PIC32Absolute::start()
{
    PR2 = timerReload;
    TMR2 = 0;
    IFS0CLR = _IFS0_T2IF_MASK;
    IEC0SET = _IEC0_T2IE_MASK;
    T2CONbits.ON = 1;
}

void PIC32Absolute::stop()
{
    T2CONbits.ON = 0;
    IEC0CLR = _IEC0_T2IE_MASK;
}

void PIC32Absolute::handleInterrupt( enum InterruptDispatcher::InterruptVector vector )
{
    IFS0CLR = _IFS0_T2IF_MASK;
    m_us += timerInterval;
}

PIC32Absolute* PIC32Absolute::getInstance()
{
    return s_instance;
}

} // namespace Timers

} // namespace Agape
