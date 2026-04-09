#ifndef AGAPE_STRATUS_HANDLER_FACTORY_H
#define AGAPE_STRATUS_HANDLER_FACTORY_H

#include "Handlers/WSRedisHandler.h"
#include "WebSockets.h"

namespace Agape
{

namespace PresenceLoaders
{
class SharedPresenceStore;
} // namespace PresenceLoaders

namespace Stratus
{

class HandlerFactory
{
public:
    HandlerFactory();

    Handler* makeHandler( ::WSTLSServer::connection_ptr connection, PresenceLoaders::SharedPresenceStore& sharedPresenceStore );

private:
    int m_clientNumber;
};

} // namespace Stratus

} // namespace Agape

#endif // AGAPE_STRATUS_HANDLER_FACTORY_H
