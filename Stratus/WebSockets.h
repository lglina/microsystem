#ifndef AGAPE_WEBSOCKETS_H
#define AGAPE_WEBSOCKETS_H

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server< websocketpp::config::asio_tls > WSTLSServer;

typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

#endif // AGAPE_WEBSOCKETS_H
