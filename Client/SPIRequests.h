#ifndef AGAPE_SPI_REQUEST_TYPES_H
#define AGAPE_SPI_REQUEST_TYPES_H

namespace Agape
{

const int maxSPIPayloadLength( 32 );

enum SPIRequestTypes
{
    SPIReadInput,
    SPIReadPowerState,
    SPIReadEntropy,
    SPISetIllumination,
    SPISetAlert,
    SPIReadSensors
};

} // namespace Agape

#endif // AGAPE_SPI_REQUEST_TYPES_H
