#include "EntropySources/EntropySource.h"
#include "SPIEntropySender.h"
#include "SPIRequests.h"

namespace Agape
{

SPIEntropySender::SPIEntropySender( EntropySource& entropySource ) :
  m_entropySource( entropySource )
{
}

int SPIEntropySender::spiResponse( char requestType,
                                   char* request,
                                   int requestLength,
                                   char* response,
                                   int maxResponseLength )
{
    if( requestType == SPIReadEntropy )
    {
        if( requestLength == 1 )
        {
            int requestedBytes = request[0];
            if( requestedBytes <= maxResponseLength )
            {
                return m_entropySource.generate( response, requestedBytes );
            }
        }
    }

    return 0;
}

} // namespace Agape
