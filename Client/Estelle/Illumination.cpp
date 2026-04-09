#include "Illumination.h"
#include "PowerControllable.h"
#include "SPIRequests.h"

#include <xc.h>

namespace
{
    const int period = 8000;
    const int defaultValue = 2000;
    const int step = 1000;
} // Anonymous namespace

namespace Agape
{

Illumination::Illumination()
{
    CCP1CON1bits.MOD = 0x05; // Dual Edge Compare mode, buffered.
    CCP1CON2bits.OCAEN = 0; // Disable A output, which is enabled by default.
    CCP1CON2bits.OCDEN = 1; // Enable D output.
    CCP1PR = period;
    CCP1RA = 0;
    CCP1RB = defaultValue;

    setPowerState( PowerState::off );
}

Illumination::~Illumination()
{
    setPowerState( PowerState::off );
}

void Illumination::run()
{
}

void Illumination::setPowerState( enum PowerState powerState )
{
    if( powerState == PowerState::on )
    {
        CCP1CON1bits.ON = 1;
    }
    else if( powerState == PowerState::off )
    {
        CCP1CON1bits.ON = 0;
    }
}

int Illumination::spiResponse( char requestType,
                               char* request,
                               int requestLength,
                               char* response,
                               int maxResponseLength )
{
    int responseLength( 0 );

    if( requestType == SPISetIlluminationUp )
    {
        if( CCP1RB <= ( period - step ) )
        {
            CCP1RB += step;
        }
        //responseLength = 0;
    }
    else if( requestType == SPISetIlluminationDown )
    {
        if( CCP1RB >= step )
        {
            CCP1RB -= step;
        }
        //responseLength = 0;
    }
    else if( requestType == SPIReadIllumination )
    {
        response[0] = CCP1RB & 0xFF;
        response[1] = ( CCP1RB >> 8 ) & 0xFF;
        responseLength = 2;
    }
    else if( requestType == SPISetIllumination )
    {
        if( requestLength == 2 )
        {
            int value( 0 );
            value = *( (unsigned char*)request );
            value += ( (int)*( (unsigned char*)( request + 1 ) ) ) << 8;

            if( ( value > 0 ) && ( value <= period ) ) // Should always be > 0, but check anyway!
            {
                CCP1RB = value;
            }
        }
        //responseLength = 0;
    }

    return responseLength;
}

} // namespace Agape
