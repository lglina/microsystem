#ifndef AGAPE_ENTROPY_SOURCES_SPI_H
#define AGAPE_ENTROPY_SOURCES_SPI_H

#include "EntropySources/EntropySource.h"
#include "Utils/RingBuffer.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class SPIRequester;
class Timer;

namespace EntropySources
{

class SPI : public EntropySource
{
public:
    SPI( SPIRequester& spiRequester,
         Timers::Factory& timerFactory );
    virtual ~SPI();

    virtual int generate( char* buffer, int len );

    virtual int poolSize();
    virtual int poolRemain();

    virtual void run();

private:
    SPIRequester& m_spiRequester;

    Timer* m_timer;

    RingBuffer< char > m_entropyPool;

    bool m_requestSent;
};

} // namespace EntropySources

} // namespace Agape

#endif // AGAPE_ENTROPY_SOURCES_SPI_H
