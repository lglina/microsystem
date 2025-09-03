#ifndef AGAPE_SPI_REQUESTER_H
#define AGAPE_SPI_REQUESTER_H

namespace Agape
{

class BusController;
class SPIController;

class SPIRequester
{
public:
    SPIRequester( SPIController& spiController,
                  BusController& bus );

    void sendRequest( char requestType,
                      char* request,
                      int requestLength );
    int readResponse( char* response );
    bool busy();

private:
    SPIController& m_spiController;
    BusController& m_bus;

    bool m_busy;
};

} // namespace Agape

#endif // AGAPE_SPI_REQUESTER_H
