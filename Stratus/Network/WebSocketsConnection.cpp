#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Utils/StrToHex.h"
#include "Stratus.h"
#include "WebSocketsConnection.h"

#include <functional>

using namespace std::placeholders;

namespace
{
    const int bufferCapacity( 32768 );
} // Anonymous namespace

namespace Agape
{

namespace Network
{

WebSocketsConnection::WebSocketsConnection( WSTLSServer::connection_ptr connection ) :
  m_connection( connection ),
  m_buffer( bufferCapacity )
{
    connection->set_message_handler( std::bind( &WebSocketsConnection::onMessage, this, _1, _2 ) );
}

WebSocketsConnection::~WebSocketsConnection()
{
    stop();
}

int WebSocketsConnection::read( char* data, int len )
{
    int bufferSize( m_buffer.size() );
    int numToRead( len > bufferSize ? bufferSize : len );
    for( int i = 0; i < numToRead; ++i )
    {
        data[i] = m_buffer.pop();
    }

#ifdef LOG_WS
    if( numToRead > 0 )
    {
        LOG_DEBUG( "WebSocketsConnection: Reading" );
        hexDump( data, numToRead );
    }
#endif

    return numToRead;
}

int WebSocketsConnection::write( const char* data, int len )
{
#ifdef LOG_WS
    //LOG_DEBUG( "QtWebSocketsConnection: Writing" );
    //hexDump( data, len );
    LiteStream stream;
    stream << "WS Write " << len;
    LOG_DEBUG( stream.str() );
    hexDump( data, len );
#endif

    m_connection->send( data, len );

    return len;
}

bool WebSocketsConnection::error()
{
    return false;
}

void WebSocketsConnection::waitIncoming()
{
    std::unique_lock< std::mutex > lock( m_mutex );
    if( m_buffer.isEmpty() )
    {
        m_bufferPending.wait( lock );
    }
}

void WebSocketsConnection::stop()
{
    // Awake all waiters.
    m_bufferPending.notify_all();
}

void WebSocketsConnection::onMessage( websocketpp::connection_hdl connectionHandle,
                                      WSTLSServer::message_ptr message )
{
    std::unique_lock< std::mutex > lock( m_mutex );

    const std::string& payload( message->get_payload() );

#ifdef LOG_WS
    LiteStream stream;
    stream << "WS Read " << (unsigned int)payload.length();
    LOG_DEBUG( stream.str() );
    hexDump( &payload[0], payload.length() );
#endif

    int numBuffered( 0 );
    while( !m_buffer.isFull() && numBuffered < payload.length() )
    {
        m_buffer.push( payload[numBuffered] );
        ++numBuffered;
    }

    if( numBuffered < payload.length() )
    {
        LOG_DEBUG( "WebSocketsConnection: Buffer overflow. Bytes lost." );
    }

    m_bufferPending.notify_all();
}

void WebSocketsConnection::sendOutOfBand( const String& message )
{
    m_connection->send( message.c_str() );
}

} // namespace Agape

} // namespace Network
