#include "InputDevices/InputDevice.h"
#include "SPIInputDeviceSender.h"
#include "SPIRequests.h"

namespace Agape
{

SPIInputDeviceSender::SPIInputDeviceSender( InputDevice& inputDevice ) :
  m_inputDevice( inputDevice )
{
}

int SPIInputDeviceSender::spiResponse( char requestType,
                                       char* request,
                                       int requestLength,
                                       char* response,
                                       int maxResponseLength )
{
    if( requestType == SPIReadInput )
    {
        int responseLength( 0 );
        while( !m_inputDevice.eof() && responseLength < maxResponseLength )
        {
            *response++ = m_inputDevice.get();
            ++responseLength;
        }

        return responseLength;
    }

    return 0;
}

} // namespace Agape
