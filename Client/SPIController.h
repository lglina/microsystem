#ifndef AGAPE_SPI_CONTROLLER_H
#define AGAPE_SPI_CONTROLLER_H

#include "Utils/RingBuffer.h"
#include "InterruptHandler.h"
#include "PowerControllable.h"
#include "ReadableWritable.h"

#include "Warp.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class Timer;

class SPIController : public InterruptHandler, public PowerControllable, public ReadableWritable
{
public:
    SPIController( int peripheral,
                   int clockSpeed,
                   bool master,
                   Timers::Factory& timerFactory,
                   int transmitBufferSize = 260,
                   int receiveBufferSize = 260 );
    ~SPIController();

    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );
    virtual bool error();

    int write( const char data );

    bool eof();

    virtual void setPowerState( enum PowerState powerState );

    virtual void handleInterrupt( enum InterruptDispatcher::InterruptVector vector );

private:
    void transmitISR();
    void receiveISR();
    inline void setRXIE();
    inline void setTXIE();
    inline void clearRXIE();
    inline void clearTXIE();
    inline void clearRXIF();
    inline void clearTXIF();
    inline void setOn( bool on );
    inline void flushInput();
    inline void clearOverflow();

    int m_peripheral;
    bool m_master;

    Timer* m_timer;

    RingBuffer< char > m_transmitBuffer;
    RingBuffer< char > m_receiveBuffer;

    volatile char m_dummyReadCount;
};

} // namespace Agape

#endif // AGAPE_SPI_CONTROLLER_H
