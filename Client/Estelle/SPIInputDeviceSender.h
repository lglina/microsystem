#ifndef AGAPE_SPI_INPUT_DEVICE_SENDER_H
#define AGAPE_SPI_INPUT_DEVICE_SENDER_H

#include "SPIResponder.h"

namespace Agape
{

class InputDevice;

class SPIInputDeviceSender : public SPIResponseSource
{
public:
    SPIInputDeviceSender( InputDevice& inputDevice );

    virtual int spiResponse( char requestType,
                             char* request,
                             int requestLength,
                             char* response,
                             int maxResponseLength );

private:
    InputDevice& m_inputDevice;
};

} // namespace Agape

#endif // AGAPE_SPI_INPUT_DEVICE_SENDER_H
