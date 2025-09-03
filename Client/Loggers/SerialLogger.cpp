#include "Loggers/SerialLogger.h"
#include "PICSerial.h"
#include "String.h"

namespace
{
    const char eol[2] = { '\r', '\n' };
}

namespace Agape
{

namespace Loggers
{

Serial::Serial( PICSerial& picSerial ) :
  m_picSerial( picSerial )
{
}

void Serial::log( const String& message, LogLevel logLevel )
{
    __builtin_disable_interrupts();
    // FIXME: Implement log levels via base class, which would call a
    // a differently-named derived class function?
    int lenWritten( -1 );
    int totalLenWritten( 0 );
    const char* dataPtr( message.c_str() );
    while( ( lenWritten != 0 ) && ( totalLenWritten < message.length() ) )
    {
        int lenRemain( message.length() - totalLenWritten );
        lenWritten = m_picSerial.write( dataPtr, lenRemain );
        dataPtr += lenWritten;
        totalLenWritten += lenWritten;
    }

    for( int i = 0; i < 2; ++i )
    {
        m_picSerial.write( eol + i, 1 );
    }

    m_picSerial.flushInput(); // In case we get any junk coming back from the monitoring PC...
    __builtin_enable_interrupts();
}

} // namespace Loggers

} // namespace Agape
