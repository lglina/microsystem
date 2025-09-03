#ifndef AGAPE_NETWORK_WEBSOCKETSCONNECTION_H
#define AGAPE_NETWORK_WEBSOCKETSCONNECTION_H

#include "Utils/RingBuffer.h"
#include "ReadableWritable.h"
#include "Stratus.h"
#include "String.h"

#include <condition_variable>
#include <mutex>

namespace Agape
{

namespace Network
{

class WebSocketsConnection : public ReadableWritable
{
public:
    WebSocketsConnection( WSTLSServer::connection_ptr connection );
    ~WebSocketsConnection();

    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );
    virtual bool error();

    void waitIncoming();

    void stop();

    void onMessage( websocketpp::connection_hdl connectionHandle,
                    WSTLSServer::message_ptr message );

    void sendOutOfBand( const String& message );

private:
    WSTLSServer::connection_ptr m_connection;

    RingBuffer< char > m_buffer;

    std::mutex m_mutex;
    std::condition_variable m_bufferPending;
};

} // namespace Agape

} // namespace Network

#endif // AGAPE_NETWORK_WEBSOCKETSCONNECTION_H
