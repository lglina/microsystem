#ifndef AGAPE_NETWORK_QT_WEBSOCKETS_CONNECTION_H
#define AGAPE_NETWORK_QT_WEBSOCKETS_CONNECTION_H

#include "Utils/RingBuffer.h"
#include "ReadableWritable.h"

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QSslError>
#include <QtWebSockets/QWebSocket>
#include <QEventLoop>
#include <QString>
#include <QTimer>

namespace Agape
{

class Platform;

namespace Network
{

class QtWebSocketsConnection : public QObject, public ReadableWritable
{
    Q_OBJECT

public:
    QtWebSocketsConnection( const QUrl& url,
                            Platform& platform,
                            QObject* parent = nullptr );
    ~QtWebSocketsConnection();

    bool isOpen();

    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );
    virtual bool error();

    void close();

public slots:
    void onConnected();
    void onDisconnected();
    void onBinaryMessageReceived( const QByteArray& message );
    void onTextMessageReceived( const QString& message );
    void onErrorOccurred( QAbstractSocket::SocketError error );
    void onSslErrors( const QList< QSslError >& errors );
    void onPong( quint64 elapsedTime, const QByteArray& payload );
    void onPongTimeout();
    void onRXTimeout();
    void exitEventLoop();
    void sendPing();

private:
    void notifyConnected();
    void notifyDisconnected();

    Platform& m_platform;

    QWebSocket m_webSocket;

    bool m_isOpen;
    bool m_error;

    RingBuffer< char > m_buffer;

    QEventLoop m_eventLoop;
    QTimer m_timer;

    QTimer m_pingTimer;
    QTimer m_pongTimer;

    QTimer m_rxTimer;
};

} // namespace Network

} // namespace Agape

#endif // AGAPE_NETWORK_QT_WEBSOCKETS_CONNECTION_H
