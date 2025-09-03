#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Utils/StrToHex.h"
#include "BusAddresses.h"
#include "BusController.h"
#include "SPIController.h"
#include "SPIRequester.h"
#include "SPIRequests.h"

#include <string.h>

#include <xc.h>

namespace Agape
{

SPIRequester::SPIRequester( SPIController& spiController,
                            BusController& bus ) :
  m_spiController( spiController ),
  m_bus( bus ),
  m_busy( false )
{
}

void SPIRequester::sendRequest( char requestType,
                                char* request,
                                int requestLength )
{
#ifdef LOG_SPI
    {
    LiteStream stream;
    stream << "SPIRequester: request type " << (int)requestType << " length " << requestLength;
    LOG_DEBUG( stream.str() );
    }
#endif

    m_bus.write( BusAddresses::CSEstelle, 0x00 );

    char requestBuffer[requestLength + 2];
    requestBuffer[0] = requestType;
    requestBuffer[1] = requestLength;
    ::memcpy( requestBuffer + 2, request, requestLength );

    m_spiController.write( requestBuffer, requestLength + 2 ); // Blocking.

    m_bus.write( BusAddresses::None, 0x00 ); // Stop asserting Estelle /CS.

    m_busy = true;
}

int SPIRequester::readResponse( char* response )
{
    m_bus.write( BusAddresses::CSEstelle, 0x00 );

    // responseLength was declared as char but this triggered a compiler bug in
    // optimised MIPS16e code where we accessed this variable to load the return
    // value CPU register AFTER restoring the stack pointer, thus allowing the
    // return value to be corrupted if we caught an interrupt at that time and
    // the interrupt pushed its own data onto the stack. Declaring this as int
    // seems to avoid the bug by (correctly) setting the return register from
    // this variable BEFORE popping the stack frame.
    // See: https://forum.microchip.com/s/topic/a5CV40000001tldMAA/t397607
    int responseLength( 0 );

    m_spiController.read( (char*)&responseLength, 1 ); // Blocking.

#ifdef LOG_SPI
    {
    LiteStream stream;
    stream << "SPIRequester: response length " << (int)responseLength;
    LOG_DEBUG( stream.str() );
    }
#endif
    
    if( ( responseLength > 0 ) && ( responseLength <= maxSPIPayloadLength ) )
    {
        m_spiController.read( response, responseLength );
    }
    else if( responseLength != 0 )
    {
        LiteStream stream;
        stream << "SPIRequester: Invalid response length " << responseLength;
        LOG_DEBUG( stream.str() );
        responseLength = 0;
    }

    m_bus.write( BusAddresses::None, 0x00 ); // Stop asserting Estelle /CS.

    m_busy = false;

    return responseLength;
}

bool SPIRequester::busy()
{
    return m_busy;
}

} // namespace Agape
