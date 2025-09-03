#include "LineDrivers/PICSerialLineDriver.h"
#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "PICSerialLineDriver.h"
#include "PICSerial.h"

namespace
{
    const int rxTimeout( 10000 ); // ms
} // Anonymous namespace

namespace Agape
{

namespace LineDrivers
{

PICSerial::PICSerial( Agape::PICSerial& picSerial, Timers::Factory& timerFactory ) :
  m_picSerial( picSerial ),
  m_rxTimer( timerFactory.makeTimer() )
{
}

PICSerial::~PICSerial()
{
    delete( m_rxTimer );
}

int PICSerial::open()
{
    LOG_DEBUG( "PICSerialLineDriver: Opening." );
    m_picSerial.flushInput();
    m_rxTimer->reset();
    return 0;
}

int PICSerial::read( char* data, int len )
{
    int numRead( m_picSerial.read( data, len ) );
    if( numRead > 0 )
    {
        m_rxTimer->reset();
    }

    return numRead;
}

int PICSerial::write( const char* data, int len )
{
    return m_picSerial.write( data, len );
}

bool PICSerial::error()
{
    // FIXME: This is a hack to get ModemLine to drop the line and force a
    // reconnect if the connection drops. Remove this when we implement
    // the DCD line etc.
    return( m_rxTimer->ms() >= rxTimeout );
}

} // namespace LineDrivers

} // namespace Agape
