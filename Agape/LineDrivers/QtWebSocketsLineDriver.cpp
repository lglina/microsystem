#include "Loggers/Logger.h"
#include "Network/QtWebSocketsConnection.h"
#include "Platforms/Platform.h"
#include "Utils/LiteStream.h"
#include "QtWebSocketsLineDriver.h"

#include <QtCore/QUrl>

namespace Agape
{

namespace LineDrivers
{

QtWebSockets::QtWebSockets( Platform& platform ) :
  m_platform( platform )
{
}

int QtWebSockets::open()
{
    LOG_DEBUG( "QtWebSocketsLineDriver: Open" );

    return 0;
}

bool QtWebSockets::isSecure()
{
    return( QUrl( m_number.c_str() ).scheme() == "wss" );
}

int QtWebSockets::read( char* data, int len )
{
    if( m_webSocketsConnection )
    {
        return( m_webSocketsConnection->read( data, len ) );
    }
    
    return 0;
}

int QtWebSockets::write( const char* data, int len )
{
    if( m_webSocketsConnection )
    {
        return( m_webSocketsConnection->write( data, len ) );
    }

    return 0;
}

bool QtWebSockets::error()
{
    if( m_webSocketsConnection )
    {
        return( m_webSocketsConnection->error() );
    }

    return false;
}

void QtWebSockets::setLinkAddress( const String& number )
{
    LiteStream stream;
    stream << "QtWebSocketsLineDriver: Set link address to " << number;
    LOG_DEBUG( stream.str() );

    m_number = number;
    m_webSocketsConnection.reset(); // Ensure previous is deleted before constructing new.
    m_webSocketsConnection.reset( new Network::QtWebSocketsConnection( QUrl( m_number.c_str() ), m_platform ) );
}

bool QtWebSockets::linkReady()
{
    if( m_webSocketsConnection )
    {
        return m_webSocketsConnection->isOpen();
    }

    return false;
}

} // namespace LineDrivers

} // namespace Agape
