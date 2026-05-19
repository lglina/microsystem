#include "LineDrivers/PICSerialLineDriver.h"
#include "Lines/Line.h"
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

void PICSerial::enableFlowControl( bool enable )
{
    m_picSerial.enableFlowControl( enable );

    if( enable )
    {
        RPB6Rbits.RPB6R = 0x01; // Map /U1RTS
    }
    else
    {
        RPB6Rbits.RPB6R = 0x00; // Unmap /U1RTS
    }
}

int PICSerial::controlLines()
{
    int controlLines( 0 );
    if( !LATCbits.LATC14 ) controlLines |= Line::DTR;
    if( !PORTCbits.RC13 ) controlLines |= Line::DCD;
    if( !LATBbits.LATB6 ) controlLines |= Line::RTS;
    if( !PORTDbits.RD9 ) controlLines |= Line::CTS;
    return controlLines;
}

void PICSerial::setControlLines( int mask )
{
    if( mask & Line::DTR )
    {
        LATCCLR = _LATC_LATC14_MASK;
    }

    if( mask & Line::RTS )
    {
        LATBCLR = _LATB_LATB6_MASK; // Presumes /RTS controlled by port, not UART peripheral.
    }
}

void PICSerial::clearControlLines( int mask )
{
    if( mask & Line::DTR )
    {
        LATCSET = _LATC_LATC14_MASK;
    }

    if( mask & Line::RTS )
    {
        LATBSET = _LATB_LATB6_MASK; // Presumes /RTS controlled by port, not UART peripheral.
    }
}

} // namespace LineDrivers

} // namespace Agape
