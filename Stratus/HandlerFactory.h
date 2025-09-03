#ifndef AGAPE_STRATUS_HANDLER_FACTORY_H
#define AGAPE_STRATUS_HANDLER_FACTORY_H

#include "Handler.h"
#include "WebSockets.h"

namespace Agape
{

namespace PresenceLoaders
{
class SharedPresenceStore;
} // namespace PresenceLoaders

namespace Stratus
{

class Hydra;

class HandlerFactory
{
public:
    HandlerFactory();

    Handler* makeHandler( ::WSTLSServer::connection_ptr connection, Hydra& hydra, PresenceLoaders::SharedPresenceStore& sharedPresenceStore );

private:
    int m_clientNumber;
};

} // namespace Stratus

} // namespace Agape

#endif // AGAPE_STRATUS_HANDLER_FACTORY_H
