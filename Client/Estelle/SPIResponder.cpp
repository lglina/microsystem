#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Collections.h"
#include "ReadableWritable.h"
#include "SPIController.h"
#include "SPIRequests.h"
#include "SPIResponder.h"
#include "Warp.h"

#include <xc.h>

namespace
{
    int readTimeout( 100 ); // ms.
} // Anonymous namespace

namespace Agape
{

SPIResponder::SPIResponder( SPIController& spiController ) :
  m_spiController( spiController ),
  m_warp( "Queueing SPI response", false )
{
}

void SPIResponder::registerResponseSource( SPIResponseSource* responseSource, char request )
{
    m_responseSources[ request ] = responseSource;
}

void SPIResponder::run()
{
    if( !m_spiController.eof() )
    {
        LATBbits.LATB7 = 1;
#ifdef LOG_SPI
        m_warp.engage();
#endif

        bool success( true );

        char requestType( -1 );
        ReadableWritable& rw( m_spiController );
        success = ( rw.read( &requestType, 1, ReadableWritable::rwBlock, readTimeout ) == 1 );

        char requestLength( 0 );
        if( success ) success = ( rw.read( &requestLength, 1, ReadableWritable::rwBlock, readTimeout ) == 1 );

#ifdef LOG_SPI
        {
        LiteStream stream;
        stream << "SPIResponder: Request type " << (int)requestType << " length " << (int)requestLength;
        LOG_DEBUG( stream.str() );
        }
#endif

        if( success && ( requestLength >= 0 ) && ( requestLength <= maxSPIPayloadLength ) )
        {
            char request[ maxSPIPayloadLength ];
            if( requestLength > 0 ) success = ( rw.read( request,
                                                         requestLength,
                                                         ReadableWritable::rwBlock,
                                                         readTimeout ) == requestLength );

            Map< char, SPIResponseSource* >::const_iterator it( m_responseSources.find( requestType ) );
            if( success && ( it != m_responseSources.end() ) )
            {
                char response[ maxSPIPayloadLength + 1 ]; // Offset 0 is response length, actual response from offset 1.
                SPIResponseSource* responseSource( it->second );
                int responseDataLength( responseSource->spiResponse( requestType,
                                                                     request,
                                                                     requestLength,
                                                                     response + 1,
                                                                     maxSPIPayloadLength ) );
                response[0] = responseDataLength;
                m_spiController.write( response, responseDataLength + 1 );

#ifdef LOG_SPI
                {
                LiteStream stream;
                stream << "SPIResponder: Response length " << responseDataLength;
                LOG_DEBUG( stream.str() );
                }
#endif
            }
            else if( success )
            {
                m_spiController.write( 0 ); // Send response length of zero.
                {
                LiteStream stream;
                stream << "SPIResponder: Request of type " << requestType << " not handled!";
                LOG_DEBUG( stream.str() );
                }
            }
            else
            {
                LOG_DEBUG( "SPIResponder: Read timeout" );
            }
        }
        else
        {
            LOG_DEBUG( "SPIResponder: Read timeout or request length invalid" );
        }

        LATBbits.LATB7 = 0;
#ifdef LOG_SPI
        m_warp.report();
#endif
    }
}

} // namespace Agape
