#ifndef AGAPE_ENTROPY_SOURCES_ESTELLE_H
#define AGAPE_ENTROPY_SOURCES_ESTELLE_H

#include "EntropySources/EntropySource.h"
#include "Utils/RingBuffer.h"
#include "PowerControllable.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class PICSerial;
class Timer;

namespace EntropySources
{

class Estelle : public EntropySource, public PowerControllable
{
public:
    Estelle( Timers::Factory& timerFactory,
             PICSerial& picSerial );
    ~Estelle();

    virtual int generate( char* buffer, int len );

    virtual void run();

    virtual void setPowerState( enum PowerState powerState );

private:
    Timer* m_timer;
    PICSerial& m_picSerial;

    RingBuffer< char > m_entropyPool;

    unsigned int m_shiftIn;
    int m_shiftCount;

    int m_count;
};

} // namespace EntropySources

} // namespace Agape

#endif // AGAPE_ENTROPY_SOURCES_ESTELLE_H
