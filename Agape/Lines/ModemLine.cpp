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
  m_isOpen( false )
{
}

void Modem::open()
{
    LOG_DEBUG( "Modem: Opening" );
    m_lineDriver.open();
    m_isOpen = true;
    m_lineDriver.dataTerminalReady( false ); // Force into command mode if in data mode.
    LOG_DEBUG( "Modem: Open" );
}

int Modem::read( char* data, int len )
{
    return( m_lineDriver.read( data, len ) );
}

int Modem::write( const char* data, int len )
{
    return( m_lineDriver.write( data, len ) );
}

Vector< Line::ConfigOption > Modem::getConfigOptions()
{
    if( !m_isOpen )
    {
        return Vector< Line::ConfigOption >();
    }

    Vector< ConfigOption > configOptions;

    String command( "AT+GCONF?\r\n" );
    m_lineDriver.flushInput();
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
    m_lineDriver.flushInput();
    m_lineDriver.write( command.c_str(), command.length() );

    LOG_DEBUG( "Set config option:" );
    LOG_DEBUG( command );

    String response;
    readLine( response ); // OK or ERROR.
}

void Modem::connect()
{
    // Connect the line, e.g. for WiFi connect to an AP but don't connect
    // to a server yet (that's what dial does).
    LOG_DEBUG( "Modem: Connecting" );
    if( !m_isOpen )
    {
        return;
    }

    // Note: We always flushInput() before write()ing commands, in case
    // we're desynchronised with the modem and an older response is already
    // in the input buffer.
    LOG_DEBUG( "Modem: Send AT+XCONN" );
    String command( "AT+XCONN\r\n" );
    m_lineDriver.flushInput();
    m_lineDriver.write( command.c_str(), command.length() );

    LOG_DEBUG( "Modem: Wait for response" );
    String response;
    readLine( response, longReadTimeout ); // OK or ERROR. Allow a longer timeout
    LOG_DEBUG( "Modem: Response: " + response );
}

void Modem::disconnect()
{
    LOG_DEBUG( "Modem: Disconnecting" );
    if( !m_isOpen )
    {
        return;
    }

    LOG_DEBUG( "Modem: Send AT+XDCON" );
    String command( "AT+XDCON\r\n" );
    m_lineDriver.flushInput();
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

    m_lineDriver.dataTerminalReady( true ); // Allow data mode.

    LiteStream commandStream;
    commandStream << "ATD" << number << "\r\n";
    String command( commandStream.str() );
    LOG_DEBUG( "Modem: Send " + command );
    m_lineDriver.flushInput();
    m_lineDriver.flushOutput(); // Flush output here to prevent AT commands ending up in the data stream.
    m_lineDriver.write( command.c_str(), command.length() );

    LOG_DEBUG( "Modem: Wait for response" );
    String response;
    readLine( response, longReadTimeout );
    LOG_DEBUG( "Modem: Response: " + response );

    if( response == "OK" && m_lineDriver.dataCarrierDetect() )
    {
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
    m_lineStatus.m_secure = false;
    m_lineDriver.dataTerminalReady( false ); // Force disconnect and back into command mode.
}

struct Line::LineStatus Modem::getLineStatus()
{
    if( !m_isOpen )
    {
        return Line::LineStatus();
    }

    m_lineStatus.m_carrier = m_lineDriver.dataCarrierDetect();
    if( !m_lineStatus.m_carrier )
    {
        m_lineStatus.m_secure = false;

        // In command mode, so we can periodically poll the line status.
        // (E.g. for WiFi, whether we're connected to an AP.)
        ++m_lineStatusCounter;
        if( m_lineStatusCounter == 1000 )
        {
            m_lineStatusCounter = 0;
            String command( "AT+XCONN?\r\n" );
            m_lineDriver.flushInput();
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
