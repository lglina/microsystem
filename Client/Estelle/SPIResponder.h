#ifndef AGAPE_SPI_RESPONDER_H
#define AGAPE_SPI_RESPONDER_H

#include "Collections.h"
#include "Runnable.h"
#include "Warp.h"

namespace Agape
{

class SPIController;

class SPIResponseSource
{
public:
    virtual int spiResponse( char requestType,
                             char* request,
                             int requestLength,
                             char* response,
                             int maxResponseLength ) = 0;
};

class SPIResponder : public Runnable
{
public:
    SPIResponder( SPIController& spiController );

    void registerResponseSource( SPIResponseSource* responseSource, char request );

    virtual void run();

private:
    SPIController& m_spiController;

    Warp m_warp;

    Map< char, SPIResponseSource* > m_responseSources;
};

} // namespace Agape

#endif // AGAPE_SPI_RESPONDER_H
