#ifndef AGAPE_SPI_ENTROPY_SENDER_H
#define AGAPE_SPI_ENTROPY_SENDER_H

#include "SPIResponder.h"

namespace Agape
{

class EntropySource;

class SPIEntropySender : public SPIResponseSource
{
public:
    SPIEntropySender( EntropySource& entropySource );

    virtual int spiResponse( char requestType,
                             char* request,
                             int requestLength,
                             char* response,
                             int maxResponseLength );

private:
    EntropySource& m_entropySource;
};

} // namespace Agape

#endif // AGAPE_SPI_ENTROPY_SENDER_H
