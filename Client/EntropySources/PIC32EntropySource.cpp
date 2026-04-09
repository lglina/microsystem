#include "PIC32EntropySource.h"

#include <xc.h>

namespace Agape
{

namespace EntropySources
{

PIC32TRNG::PIC32TRNG()
{
    RNGCONbits.TRNGEN = 1;
    RNGCONbits.TRNGMODE = 1; // Enable bias corrector.
}

int PIC32TRNG::generate( char* buffer, int len )
{
    for( int i = 0; i < len; ++i )
    {
        while( RNGCNT < 8 ) {}
        *(unsigned char*)( buffer + i ) = RNGSEED1;
    }

    return len;
}

int PIC32TRNG::poolSize()
{
    return 256;
}

int PIC32TRNG::poolRemain()
{
    return 256;
}

void PIC32TRNG::run()
{
    // NOP
}

} // namespace EntropySources

} // namespace Agape
