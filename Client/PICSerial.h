#ifndef AGAPE_PIC_SERIAL_H
#define AGAPE_PIC_SERIAL_H

#include "Utils/RingBuffer.h"
#include "InterruptHandler.h"
#include "ReadableWritable.h"
#include <xc.h>

namespace Agape
{

class PICSerial : public InterruptHandler, public ReadableWritable
{
public:
    PICSerial( int port,
               int baud,
               int transmitBufferSize = 256,
               int receiveBufferSize = 1024,
               bool flowControl = false,
               bool debug = false );

    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );

    virtual bool error();

    virtual void flushInput();

    int write( const char datum );

    bool eof() const;

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

    void doLog();

    int m_port;

    RingBuffer< char > m_transmitBuffer;
    RingBuffer< char > m_receiveBuffer;

    bool m_flowControl;

    int m_transmitHighWater;
    int m_transmitBufferFulls;
    int m_receiveHighWater;
    int m_receiveFIFOOverflows;
    int m_receiveBufferOverflows;
    int m_receiveErrors;

    bool m_debug;
    volatile bool m_needLog;
};

} // namespace Agape

#endif // AGAPE_PIC_SERIAL_H
