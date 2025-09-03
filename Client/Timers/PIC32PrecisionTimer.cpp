#include "PIC32PrecisionTimer.h"
#include "cpu.h"

#include <xc.h>

namespace
{
    const unsigned long ticksPerUS( PBCLK_FREQ / 1000000ul );
    //const int constOffset( 70 ); // Ticks to subtract from delay times, accounting for all overheads.
    const int constOffset( 0 ); // Ticks to subtract from delay times, accounting for all overheads.
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
    long ticks( us * ticksPerUS );
    if( ticks > constOffset ) ticks -= constOffset; // FIXME: Need to verify fixed offset again at 8 and 32 MHz.
    T3CONbits.ON = 0;
    TMR3 = 0;
    T3CONbits.ON = 1;
    while( TMR3 < ticks ) {}
}

} // namespace Timers

} // namespace Agape
