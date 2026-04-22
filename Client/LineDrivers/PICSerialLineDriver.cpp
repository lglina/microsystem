#include "LineDrivers/PICSerialLineDriver.h"
#include "Loggers/Logger.h"
#include "PICSerialLineDriver.h"
#include "PICSerial.h"

#include <xc.h>

namespace
{
    const int rxTimeout( 10000 ); // ms
} // Anonymous namespace

namespace Agape
{

namespace LineDrivers
{

PICSerial::PICSerial( Agape::PICSerial& picSerial ) :
  m_picSerial( picSerial )
{
}

int PICSerial::open()
{
    LOG_DEBUG( "PICSerialLineDriver: Opening." );
    m_picSerial.flushInput();
    return 0;
}

int PICSerial::read( char* data, int len )
{
    return( m_picSerial.read( data, len ) );
}

int PICSerial::write( const char* data, int len )
{
    return m_picSerial.write( data, len );
}

bool PICSerial::error()
{
    // We're being used by ReadableWritableTupleRoute. Setting error when
    // carrier is dropped indicates to TupleRouter (via TupleRoute) that
    // there's no point sending anything else until we reconnect. This avoids
    // long lockups for the user and lets the user run offline/disconnected.
    return !dataCarrierDetect();
}

void PICSerial::flushInput()
{
    m_picSerial.flushInput();
}

void PICSerial::flushOutput()
{
    m_picSerial.flushOutput();
}

bool PICSerial::dataCarrierDetect()
{
    // FIXME: Should DCD and DTR functions be moved into PICSerial.c?
    // Unlike RTS/CTS flow control they're not part of the PIC UART peripheral,
    // so perhaps here really is the best place for them.
    return !PORTCbits.RC13;
}

void PICSerial::dataTerminalReady( bool ready )
{
    if( ready )
    {
        PORTCCLR = _PORTC_RC14_MASK;
    }
    else
    {
        PORTCSET = _PORTC_RC14_MASK;
    }
}

} // namespace LineDrivers

} // namespace Agape
