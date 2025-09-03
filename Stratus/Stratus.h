#ifndef AGAPE_STRATUS_H
#define AGAPE_STRATUS_H

#include "PresenceLoaders/SharedPresenceStore.h"
#include "HandlerFactory.h"
#include "Hydra.h"
#include "MasterClock.h"
#include "WebSockets.h"

#include "Timers/Factories/TimerFactory.h"

#include <map>
#include <memory>
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

    int m_port;

    WSTLSServer m_wsEndpoint;

    Hydra m_hydra;

    PresenceLoaders::SharedPresenceStore m_sharedPresenceStore;

    HandlerFactory m_handlerFactory;

    std::map< websocketpp::connection_hdl, Handler*, std::owner_less< websocketpp::connection_hdl > > m_handlers;

    MasterClock m_masterClock;
    std::unique_ptr< std::thread > m_clockThread;
    bool m_stopping;

    Timers::Factory* m_timerFactory;
    Timers::Factory* m_performanceTimerFactory;
};

} // namespace Stratus

} // namespace Agape

#endif // AGAPE_STRATUS_H
