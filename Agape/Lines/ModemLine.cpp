#include "Collections.h"
#include "Lines/Line.h"
#include "Lines/ModemLine.h"
#include "LineDrivers/LineDriver.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Utils/Tokeniser.h"
#include "String.h"

#include <stddef.h>

#include "Loggers/Logger.h"

namespace
{
    const int longReadTimeout( 30000 ); // ms. Longer timeout for slow operations.
}

namespace Agape
{

namespace Lines
{

Modem::Modem( LineDriver& lineDriver ) :
  Line( lineDriver ),
  m_isOpen( false ),
  m_txCounter( 0 ),
  m_rxCounter( 0 )
{
}

void Modem::open()
{
    LOG_DEBUG( "Modem: Opening" );
    m_lineDriver.open();
    m_isOpen = true;
    LOG_DEBUG( "Modem: Open" );
}

int Modem::read( char* data, int len )
{
    int numRead( m_lineDriver.read( data, len ) );
    /*
    if( numRead > 0 )
    {
        m_rxCounter += numRead;
        LiteStream stream;
        stream << "RX: " << m_rxCounter;
        LOG_DEBUG( stream.str() );
    }
    */
    return numRead;
}

int Modem::write( const char* data, int len )
{
    int numWritten( m_lineDriver.write( data, len ) );
    /*
    if( numWritten > 0 )
    {
        m_txCounter += numWritten;
        LiteStream stream;
        stream << "TX: " << m_txCounter;
        LOG_DEBUG( stream.str() );
    }
    */
    return numWritten;
}

Vector< Line::ConfigOption > Modem::getConfigOptions()
{
    if( !m_isOpen )
    {
        return Vector< Line::ConfigOption >();
    }

    Vector< ConfigOption > configOptions;

    String command( "AT+GCONF?\r\n" );
    m_lineDriver.write( command.c_str(), command.length() );

    String response;
    readLine( response );
    while( !response.empty() && ( response != "END" ) )
    {
        ConfigOption thisOption;
        Tokeniser tokeniser( response, ',' );

        thisOption.m_name = tokeniser.token();

        String typeStr( tokeniser.token() );
        if( typeStr == "text" )
        {
            thisOption.m_type = text;
        }
        else if( typeStr == "encodedText" )
        {
            thisOption.m_type = encodedText;
        }
        else if( typeStr == "select" )
        {
            thisOption.m_type = select;
        }

        thisOption.m_alternatives = tokeniser.token();

        configOptions.push_back( thisOption );

        readLine( response );
    }

    return configOptions;
}

void Modem::setConfigOption( const String& name, const String& value )
{
    if( !m_isOpen )
    {
        return;
    }

    LiteStream commandStream;
    commandStream << "AT+GCONF=" << name << "=" << value << "\r\n";
    String command( commandStream.str() );
    m_lineDriver.write( command.c_str(), command.length() );

    LOG_DEBUG( "Set config option:" );
    LOG_DEBUG( command );

    String response;
    readLine( response ); // OK or ERROR.
}

void Modem::connect()
{
    LOG_DEBUG( "Modem: Connecting" );
    if( !m_isOpen )
    {
        return;
    }

    LOG_DEBUG( "Modem: Send AT+XCONN" );
    String command( "AT+XCONN\r\n" );
    m_lineDriver.write( command.c_str(), command.length() );

    LOG_DEBUG( "Modem: Wait for response" );
    String response;
    readLine( response, longReadTimeout ); // OK or ERROR. Allow a longer timeout
    LOG_DEBUG( "Modem: Response: " + response );
}

void Modem::registerNumber( const String& number )
{
    // FIXME: Stub.
}

void Modem::dial( const String& number )
{
    if( !m_isOpen )
    {
        return;
    }

    LiteStream commandStream;
    commandStream << "ATD" << number << "\r\n";
    String command( commandStream.str() );
    LOG_DEBUG( "Modem: Send " + command );
    m_lineDriver.write( command.c_str(), command.length() );

    LOG_DEBUG( "Modem: Wait for response" );
    String response;
    readLine( response, longReadTimeout );
    LOG_DEBUG( "Modem: Response: " + response );

    if( response == "OK" )
    {
        // TODO: Use DCD input.
        m_lineStatus.m_carrier = true;
        m_lineStatus.m_secure = ( number.substr( 0, 3 ) == "wss" );
    }
}

void Modem::answer()
{
    // FIXME: Stub.
}

void Modem::hangup()
{
    // FIXME: Stub.
    // TODO: How do we signal this?
}

struct Line::LineStatus Modem::getLineStatus()
{
    if( !m_isOpen )
    {
        return Line::LineStatus();
    }

    // FIXME: Detect carrier drop with DCD input and reset m_carrier (and m_secure).
    if( !m_lineStatus.m_carrier )
    {
        ++m_lineStatusCounter;
        if( m_lineStatusCounter == 1000 )
        {
            m_lineStatusCounter = 0;
            String command( "AT+XCONN?\r\n" );
            m_lineDriver.write( command.c_str(), command.length() );

            String response;
            readLine( response );
            if( response == "CONNECTED" )
            {
                m_lineStatus.m_ready = true;
            }
            else
            {
                m_lineStatus.m_ready = false;
            }
        }
    }
    else if( m_lineDriver.error() )
    {
        m_isOpen = false;
    }

    return m_lineStatus;
}

} // namespace Lines

} // namespace Agape
