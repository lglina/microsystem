#include "Loggers/Logger.h"
#include "Platforms/Platform.h"
#include "Utils/LiteStream.h"
#include "Utils/StrToHex.h"
#include "QtWebSocketsConnection.h"

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QSslError>
#include <QtWebSockets/QWebSocket>
#include <QWebSocketHandshakeOptions>
#include <QEventLoop>
#include <QTimer>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <string.h>
#endif

namespace
{
    const int bufferSize( 1048576 );

    const int pingInterval( 30000 ); // ms
    const int pongTimeout( 10000 ); // ms

    const int rxTimeout( 30000 ); // ms
}

namespace Agape
{

namespace Network
{

QtWebSocketsConnection::QtWebSocketsConnection( const QUrl& url,
                                                Platform& platform,
                                                QObject* parent ) :
  QObject( parent ),
  m_platform( platform ),
  m_isOpen( false ),
  m_error( false ),
  m_buffer( bufferSize )
{
    LOG_DEBUG( "QtWebSocketsConnection: Connecting signals" );
    connect( &m_webSocket,
             &QWebSocket::connected,
             this,
             &QtWebSocketsConnection::onConnected );

#ifndef __EMSCRIPTEN__
    connect( &m_webSocket,
             QOverload< const QList< QSslError >& >::of( &QWebSocket::sslErrors ),
             this,
             &QtWebSocketsConnection::onSslErrors );
#endif

    connect( &m_webSocket,
             QOverload< QAbstractSocket::SocketError >::of( &QWebSocket::errorOccurred ),
             this,
             &QtWebSocketsConnection::onErrorOccurred );

    connect( &m_timer, &QTimer::timeout, this, &QtWebSocketsConnection::exitEventLoop );

    // Set connection to error state if pong not received 10s after ping.
    // Sending a ping is not supported in Emscripten/Javascript, though, so
    // instead go to error if no data received for 10s (we should at least
    // receive clock ticks from the server once a second).
#ifndef __EMSCRIPTEN__
    connect( &m_pongTimer, &QTimer::timeout, this, &QtWebSocketsConnection::onPongTimeout );
#else
    connect( &m_rxTimer, &QTimer::timeout, this, &QtWebSocketsConnection::onRXTimeout );
#endif

    LOG_DEBUG( "QtWebSocketsConnection: Opening" );
    QWebSocketHandshakeOptions handshakeOptions;
    QString subprotocol( "Linda2" );
    handshakeOptions.setSubprotocols( QStringList( subprotocol ) );
    m_webSocket.open( url, handshakeOptions );

    // All our code *really* should be written such that everything is
    // asynchronous and we can return to the Qt main loop whenever we're waiting
    // for stuff from the network. We'll do that in future, but for now it's
    // too hard to refactor everything to save state part-way through. As a
    // hack, we use a nested event loop, to hand back control to Qt so we can
    // send/receive on the network (and receive signals from our QWebSocket).

    // It used to be that for WebAssembly we needed to use a zero timer as
    // processEvents() didn't work properly (see QTBUG-120570), or in earlier
    // versions of the port, even a non-zero timer. As of Qt 6.10 beta, event
    // loop stuff has been completely refactored and so a call to
    // processEvents() should be sufficient in all cases, except where we
    // deliberately want to wait longer to avoid burning CPU (see below
    // in read()).
}

QtWebSocketsConnection::~QtWebSocketsConnection()
{
    close();
}

bool QtWebSocketsConnection::isOpen()
{
    if( !m_isOpen )
    {
        // Go to the event loop to give QWebSocket a chance to connect.
        m_eventLoop.processEvents();
    }
    return m_isOpen;
}

int QtWebSocketsConnection::read( char* data, int len )
{
    if( !m_isOpen || m_error )
    {
        // Make reading while in error or not open a permanent error.
        // QtWebSocketsLineDriver will destruct this object and recreate
        // to re-open the connection (and m_error constructs false).
        m_error = true;
        return 0;
    }

    int bufferSize( m_buffer.size() );

    if( bufferSize < len )
    {
        // Only enter nested event loop if we don't have enough data waiting.
        m_eventLoop.processEvents();
        bufferSize = m_buffer.size();

#ifndef __EMSCRIPTEN__
        // If we yielded to the event loop and there weren't data waiting,
        // wait a little longer, else we'll return to the caller who will call
        // read() again immediately, and that burns the CPU.
        // Note: This seems to cause issues with wasm/asyncify!
        if( bufferSize < len )
        {
            m_timer.setSingleShot( true );
            m_timer.start( 1 );
            m_eventLoop.exec();
        }
#endif

        bufferSize = m_buffer.size();
    }

    int numToRead( len > bufferSize ? bufferSize : len );
    for( int i = 0; i < numToRead; ++i )
    {
        data[i] = m_buffer.pop();
    }

    return numToRead;
}

int QtWebSocketsConnection::write( const char* data, int len )
{
    if( !m_isOpen || m_error )
    {
        // Make writing while in error or not open a permanent error.
        // QtWebSocketsLineDriver will destruct this object and recreate
        // to re-open the connection (and m_error constructs false).
        m_error = true;
        return 0;
    }

    m_webSocket.sendBinaryMessage( QByteArray( data, len ) );
    m_webSocket.flush();

#ifdef LOG_WS
    LOG_DEBUG( "QtWebSocketsConnection: Writing" );
    hexDump( data, len );
#endif

    while( m_isOpen &&
           !m_error &&
           m_webSocket.bytesToWrite() != 0 )
    {
        m_eventLoop.processEvents();
    }

    return len;
}

bool QtWebSocketsConnection::error()
{
    return m_error;
}

void QtWebSocketsConnection::close()
{
    LOG_DEBUG( "QtWebSocketsConnection: Closing" );
    m_isOpen = false;
    m_pingTimer.stop();
    m_webSocket.close();
}

void QtWebSocketsConnection::onConnected()
{
    LOG_DEBUG( "QtWebSocketsConnection: Connected" );

    connect( &m_webSocket,
             &QWebSocket::disconnected,
             this,
             &QtWebSocketsConnection::onDisconnected );
    
    connect( &m_webSocket,
             &QWebSocket::binaryMessageReceived,
             this,
             &QtWebSocketsConnection::onBinaryMessageReceived );

    connect( &m_webSocket,
             &QWebSocket::textMessageReceived,
             this,
             &QtWebSocketsConnection::onTextMessageReceived );

    connect( &m_webSocket,
             &QWebSocket::pong,
             this,
             &QtWebSocketsConnection::onPong );

#ifndef __EMSCRIPTEN__
    connect( &m_pingTimer, &QTimer::timeout, this, &QtWebSocketsConnection::sendPing );
    m_pingTimer.start( pingInterval );
#else
    m_rxTimer.setSingleShot( true );
    m_rxTimer.start( rxTimeout );
#endif

    notifyConnected();

    m_isOpen = true;
}

void QtWebSocketsConnection::onDisconnected()
{
    LOG_DEBUG( "QtWebSocketsConnection: Disconnected" );

    m_isOpen = false;

    m_pingTimer.stop();
    m_rxTimer.stop();

    notifyDisconnected();
}

void QtWebSocketsConnection::onBinaryMessageReceived( const QByteArray& message )
{
#ifdef __EMSCRIPTEN__
    // Don't buffer time tuples for inactive tabs, or else m_buffer
    // will overflow!
    if( ( message.length() == 0x47 ) &&
        ( String( message.data() + 0x43, 4 ) == "Time" ) &&
        ( ::strcmp( emscripten_run_script_string( "if(document.hidden) \"hidden\"" ), "hidden" ) == 0 ) )
    {
        m_rxTimer.stop();
        m_rxTimer.setSingleShot( true );
        m_rxTimer.start( rxTimeout );
        return;
    }
#endif

    int bufferFree( m_buffer.free() );
    int numToBuffer( bufferFree < message.length() ? bufferFree : message.length() );
    for( int i = 0; i < numToBuffer; ++i )
    {
        m_buffer.push( message[i] );
    }

#ifdef LOG_WS
    LOG_DEBUG( "QtWebSocketsConnection: Reading" );
    hexDump( message, message.length() );
#endif

    if( numToBuffer < message.length() )
    {
        LOG_DEBUG( "QtWebSocketsConnection: Buffer overflow. Bytes lost." );
    }

#ifdef __EMSCRIPTEN__
    if( ::strcmp( emscripten_run_script_string( "if(document.hidden) \"hidden\"" ), "hidden" ) == 0 )
    {
        LiteStream stream;
        stream << "QtWebSocketsConnection: Buffer fill now " << m_buffer.size() << "/" << bufferSize << " for inactive tab.";
        LOG_DEBUG( stream.str() );
    }

    m_rxTimer.stop();
    m_rxTimer.setSingleShot( true );
    m_rxTimer.start( rxTimeout );
#endif

    // If we were in the nested event loop, break out now so read() can return.
    m_eventLoop.exit();
}

void QtWebSocketsConnection::onTextMessageReceived( const QString& message )
{
    if( message == "notification.chat" )
    {
        LOG_DEBUG( "QtWebSocketsConnection: Received chat push notification" );
        m_platform.notify( Platform::newChat, Platform::server );
    }
    else if( message == "notification.telegram" )
    {
        LOG_DEBUG( "QtWebSocketsConnection: Received telegram push notification" );
        m_platform.notify( Platform::newTelegram, Platform::server );
    }
    else
    {
        LOG_DEBUG( "QtWebSocketsConnection: Received unknown push notification" );
    }
}

void QtWebSocketsConnection::onErrorOccurred( QAbstractSocket::SocketError error )
{
    LOG_DEBUG( "QtWebSocketsConnection: Socket error" );
    m_isOpen = false;
    m_error = true;

    notifyDisconnected();
}

void QtWebSocketsConnection::onSslErrors( const QList< QSslError >& errors )
{
    LOG_DEBUG( "QtWebSocketsConnection: SSL error" );

    m_isOpen = false;
    m_error = true;
    //m_webSocket.ignoreSslErrors(); // DANGER!
}

void QtWebSocketsConnection::onPong( quint64 elapsedTime, const QByteArray& payload )
{
    LOG_DEBUG( "Pong" );
    m_pongTimer.stop();
}

void QtWebSocketsConnection::onPongTimeout()
{
    LOG_DEBUG( "Pong timeout" );
    m_isOpen = false;
    m_error = true;
}

void QtWebSocketsConnection::onRXTimeout()
{
    LOG_DEBUG( "RX timeout" );
    m_isOpen = false;
    m_error = true;

    notifyDisconnected();
}

void QtWebSocketsConnection::exitEventLoop()
{
    m_eventLoop.exit();
}

void QtWebSocketsConnection::sendPing()
{
    LOG_DEBUG( "Ping" );

    m_pongTimer.setSingleShot( true );
    m_pongTimer.start( pongTimeout );

    m_webSocket.ping();
    m_webSocket.flush();

    while( m_webSocket.bytesToWrite() != 0 )
    {
        m_eventLoop.processEvents();
    }
}

void QtWebSocketsConnection::notifyConnected()
{
    m_platform.cancelNotify( Platform::connectionError );
}

void QtWebSocketsConnection::notifyDisconnected()
{
    m_platform.notify( Platform::connectionError, Platform::server );
}

} // namespace Network

} // namespace Agape
