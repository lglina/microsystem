#include "Illumination.h"
#include "PowerControllable.h"
#include "SPIRequests.h"

#include <xc.h>

namespace Agape
{

Illumination::Illumination()
{
    CCP1CON1bits.MOD = 0x05;
    CCP1CON2bits.OCAEN = 0;
    CCP1CON2bits.OCDEN = 1; // Enable D output.
    CCP1PR = 8000; // Period.
    CCP1RA = 0;
    CCP1RB = 2000;
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
    if( requestType == SPISetIllumination )
    {
        if( requestLength == 2 )
        {
            int count = *( (unsigned char*)request );
            count += ( *( (unsigned char*)( request + 1 ) ) ) << 8;
            response[0] = 0x00;
            return 1;
        }
    }

    return 0;
}

} // namespace Agape
