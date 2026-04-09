#include "PIC32PrecisionTimer.h"
#include "cpu.h"

#include <xc.h>

namespace
{
    const unsigned long ticksPerUS( PBCLK_FREQ / 1000000ul );

    // Ticks to subtract from delay times in usleep(), accounting for all
    // overheads. This implies the minimum possible delay is 11.25 us at
    // 8 MHz, or 2.8125 us at 32 MHz.
    const int constOffset( 90 );
} // Anonymous namespace

namespace Agape
{

namespace Timers
{

PIC32Precision::PIC32Precision()
{
    reset();
}

long PIC32Precision::ms()
{
    return( us() / 1000 );
}

long PIC32Precision::us()
{
    return( TMR3 / ticksPerUS ); // 16-bit timer, so we're good to convert to signed here.
}

void PIC32Precision::reset()
{
    T3CONbits.ON = 0;
    TMR3 = 0;
    T3CONbits.ON = 1;
}

void PIC32Precision::usleep( long us )
{
    // Calculate sleep time.
    long sleepTicks( us * ticksPerUS );
    sleepTicks -= constOffset;
    if( sleepTicks < 0 ) sleepTicks = 0;

    // Save current ticks and reset.
    long prevTicks = TMR3;
    T3CONbits.ON = 0;
    TMR3 = 0;
    T3CONbits.ON = 1;

    while( TMR3 < sleepTicks ) {}

    // Restore previous ticks.
    T3CONbits.ON = 0;
    TMR3 = prevTicks + sleepTicks;
    T3CONbits.ON = 1;
}

} // namespace Timers

} // namespace Agape
