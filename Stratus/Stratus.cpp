#include "Loggers/Logger.h"
#include "Timers/Factories/CTimerFactory.h"
#include "Timers/Factories/HighResTimerFactory.h"
#include "HandlerFactory.h"
#include "Handler.h"
#include "MasterClock.h"
#include "ReadableWritable.h"
#include "Stratus.h"
#include "Warp.h"
#include "WebSockets.h"

#include <unistd.h>

#include <functional>
#include <memory>
#include <string>
#include <thread>

using namespace std::placeholders;

namespace Agape
{

namespace Stratus
{

Stratus::Stratus( int port ) :
  m_port( port ),
  m_masterClock( m_hydra ),
  m_stopping( false )
{
    // FIXME: We also build a timer factory in each handler - could probably
    // have this and other shared things in here and pass them to makeHandler()
    // on the handler factory.
    m_timerFactory = new Timers::Factories::C;
    ReadableWritable::setTimerFactory( m_timerFactory );
    m_performanceTimerFactory = new Timers::Factories::HighRes;
    Warp::setTimerFactory( m_performanceTimerFactory );

    m_wsEndpoint.set_error_channels( websocketpp::log::elevel::all );
    m_wsEndpoint.set_access_channels( websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload );

    m_wsEndpoint.clear_error_channels( websocketpp::log::elevel::all );
    m_wsEndpoint.clear_access_channels( websocketpp::log::alevel::all );
    
    m_wsEndpoint.init_asio();

    m_wsEndpoint.set_tcp_pre_bind_handler( std::bind( &Stratus::onTCPPreBind, this, _1 ) );
    m_wsEndpoint.set_socket_init_handler( std::bind( &Stratus::onSocketInit, this, _1, _2 ) );
    m_wsEndpoint.set_tls_init_handler( std::bind( &Stratus::onTLSInit, this, _1 ) );
    m_wsEndpoint.set_validate_handler( std::bind( &Stratus::onValidate, this, _1 ) );
    m_wsEndpoint.set_open_handler( std::bind( &Stratus::onOpen, this, _1 ) );
    m_wsEndpoint.set_fail_handler( std::bind( &Stratus::onFail, this, _1 ) );
    m_wsEndpoint.set_close_handler( std::bind( &Stratus::onClose, this, _1 ) );
}

Stratus::~Stratus()
{
    auto it( m_handlers.begin() );
    for( ; it != m_handlers.end(); ++it )
    {
        delete( it->second );
        m_handlers.erase( it );
    }

    m_stopping = true;
    m_clockThread->join();
}

void Stratus::run()
{
    LOG_DEBUG( "Stratus: Run" );

    m_clockThread.reset( new std::thread( std::bind( &Stratus::runClock, this ) ) );

    m_wsEndpoint.listen( m_port );

    m_wsEndpoint.start_accept();

    m_wsEndpoint.run();
}

websocketpp::lib::error_code Stratus::onTCPPreBind( websocketpp::lib::shared_ptr< websocketpp::lib::asio::ip::tcp::acceptor > acceptorPtr )
{
    websocketpp::lib::asio::socket_base::reuse_address option( true );
    acceptorPtr->set_option( option );
    return websocketpp::lib::error_code{};
}

void Stratus::onSocketInit( websocketpp::connection_hdl connectionHandle,
                            websocketpp::lib::asio::ssl::stream< websocketpp::lib::asio::ip::tcp::socket >& socket )
{
    LOG_DEBUG( "Stratus: Socket init" );

    // Due to bug in websocketpp, this doesn't work right now, pending a fix
    // being merged. It doesn't appear to make much difference anyway.
    //websocketpp::lib::asio::ip::tcp::no_delay option( true );
    //socket.lowest_layer().set_option( option );
}

context_ptr Stratus::onTLSInit( websocketpp::connection_hdl hdl)
{
    LOG_DEBUG( "Stratus: TLS init" );

    namespace asio = websocketpp::lib::asio;

    context_ptr ctx = websocketpp::lib::make_shared< asio::ssl::context >( asio::ssl::context::sslv23 );

    try
    {
        ctx->set_options(asio::ssl::context::default_workarounds |
                                asio::ssl::context::no_sslv2 |
                                asio::ssl::context::no_sslv3 |
                                asio::ssl::context::single_dh_use);
        ctx->use_certificate_chain_file("cert.pem");
        ctx->use_private_key_file("key.pem", asio::ssl::context::pem);
        ctx->set_options(asio::ssl::context::default_workarounds |
                                asio::ssl::context::no_sslv2 |
                                asio::ssl::context::no_sslv3 |
                                asio::ssl::context::single_dh_use);
        ctx->set_verify_mode(asio::ssl::verify_none);
        std::string ciphers( "ECDHE-RSA-AES128-GCM-SHA256:\
                            ECDHE-ECDSA-AES128-GCM-SHA256:\
                            ECDHE-RSA-AES256-GCM-SHA384:\
                            ECDHE-ECDSA-AES256-GCM-SHA384:\
                            DHE-RSA-AES128-GCM-SHA256:\
                            DHE-DSS-AES128-GCM-SHA256:\
                            kEDH+AESGCM:\
                            ECDHE-RSA-AES128-SHA256:\
                            ECDHE-ECDSA-AES128-SHA256:\
                            ECDHE-RSA-AES128-SHA:\
                            ECDHE-ECDSA-AES128-SHA:\
                            ECDHE-RSA-AES256-SHA384:\
                            ECDHE-ECDSA-AES256-SHA384:\
                            ECDHE-RSA-AES256-SHA:\
                            ECDHE-ECDSA-AES256-SHA:\
                            DHE-RSA-AES128-SHA256:\
                            DHE-RSA-AES128-SHA:\
                            DHE-DSS-AES128-SHA256:\
                            DHE-RSA-AES256-SHA256:\
                            DHE-DSS-AES256-SHA:\
                            DHE-RSA-AES256-SHA:\
                            AES128-GCM-SHA256:\
                            AES256-GCM-SHA384:\
                            AES128-SHA256:\
                            AES256-SHA256:\
                            AES128-SHA:\
                            AES256-SHA:\
                            AES:\
                            CAMELLIA:\
                            DES-CBC3-SHA:\
                            !aNULL:\
                            !eNULL:\
                            !EXPORT:\
                            !DES:\
                            !RC4:\
                            !MD5:\
                            !PSK:\
                            !aECDH:\
                            !EDH-DSS-DES-CBC3-SHA:\
                            !EDH-RSA-DES-CBC3-SHA:\
                            !KRB5-DES-CBC3-SHA" );
        
        if( SSL_CTX_set_cipher_list (ctx->native_handle(), ciphers.c_str() ) != 1 )
        {
            LOG_DEBUG( "Stratus: onTLSInit: Error setting cipher list" );
        }
    }
    catch( std::exception& e )
    {
        LOG_DEBUG( "TLS init exception" );
    }

    return ctx;
}

bool Stratus::onValidate( websocketpp::connection_hdl connectionHandle )
{
    LOG_DEBUG( "Stratus: Connection validate" );

    WSTLSServer::connection_ptr connection( m_wsEndpoint.get_con_from_hdl( connectionHandle ) );
    const std::vector< std::string >& subprotocols( connection->get_requested_subprotocols() );
    for( auto protocol : subprotocols )
    {
        if( protocol == "Linda2" )
        {
            LOG_DEBUG( "Stratus: Found subprotocol Linda2. Validation successful." );
            connection->select_subprotocol( "Linda2" );
            return true;
        }
    }

    LOG_DEBUG( "Stratus: Missing subprotocol Linda2. Validation failed." );
    connection->set_status( websocketpp::http::status_code::bad_request );
    return false;
}

void Stratus::onOpen( websocketpp::connection_hdl connectionHandle )
{
    LOG_DEBUG( "Stratus: Connection open" );

    WSTLSServer::connection_ptr connection( m_wsEndpoint.get_con_from_hdl( connectionHandle ) );

    LOG_DEBUG( "Creating handler" );
    Handler* handler( m_handlerFactory.makeHandler( connection, m_hydra, m_sharedPresenceStore ) );
    m_handlers[connectionHandle] = handler;
    handler->handle();
}

void Stratus::onFail( websocketpp::connection_hdl connectionHandle )
{
    LOG_DEBUG( "Stratus: Connection failed" );

    WSTLSServer::connection_ptr connection( m_wsEndpoint.get_con_from_hdl( connectionHandle ) );

    websocketpp::lib::asio::error_code ec( connection->get_transport_ec() );
}

void Stratus::onClose( websocketpp::connection_hdl connectionHandle )
{
    LOG_DEBUG( "Stratus: Connection close" );

    auto it( m_handlers.find( connectionHandle ) );
    if( it != m_handlers.end() )
    {
        delete( it->second );
        m_handlers.erase( it );
    }
}

void Stratus::runClock()
{
    while( !m_stopping )
    {
        m_masterClock.run();
        usleep( 100000 );
    }
}

} // namespace Stratus

} // namespace Agape
