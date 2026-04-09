#ifndef AGAPE_STRATUS_H
#define AGAPE_STRATUS_H

#include "Handlers/Factories/WSRedisHandlerFactory.h"
#include "PresenceLoaders/SharedPresenceStore.h"
#include "RedisMasterClock.h"
#include "WebSockets.h"

#include "Timers/Factories/TimerFactory.h"

#include <map>
#include <memory>
#include <mutex>
#include <thread>

namespace Agape
{

namespace Stratus
{

class Handler;

class Stratus
{
public:
    Stratus( int port );
    ~Stratus();

    void run();

    websocketpp::lib::error_code onTCPPreBind( websocketpp::lib::shared_ptr< websocketpp::lib::asio::ip::tcp::acceptor > acceptorPtr );
    void onSocketInit( websocketpp::connection_hdl connectionHandle,
                       websocketpp::lib::asio::ssl::stream< websocketpp::lib::asio::ip::tcp::socket >& socket );
    context_ptr onTLSInit( websocketpp::connection_hdl hdl);
    bool onValidate( websocketpp::connection_hdl connectionHandle );
    void onOpen( websocketpp::connection_hdl connectionHandle );
    void onFail( websocketpp::connection_hdl connectionHandle );
    void onClose( websocketpp::connection_hdl connectionHandle );

private:
    void runClock();
    void runStats();

    int m_port;

    WSTLSServer m_wsEndpoint;

    PresenceLoaders::SharedPresenceStore m_sharedPresenceStore;

    HandlerFactory m_handlerFactory;

    std::map< websocketpp::connection_hdl, Handler*, std::owner_less< websocketpp::connection_hdl > > m_handlers;

    MasterClock m_masterClock;
    std::unique_ptr< std::thread > m_clockThread;
    bool m_stopping;

    Timers::Factory* m_timerFactory;
    Timers::Factory* m_performanceTimerFactory;

    std::mutex m_handlersMutex;
    int m_failTLS;
    int m_failValidate;
    int m_fail;
    std::unique_ptr< std::thread > m_statsThread;
};

} // namespace Stratus

} // namespace Agape

#endif // AGAPE_STRATUS_H
