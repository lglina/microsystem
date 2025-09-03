#ifndef AGAPE_PRESENCE_LOADERS_FACTORIES_OFFLINE_H
#define AGAPE_PRESENCE_LOADERS_FACTORIES_OFFLINE_H

#include "PresenceLoadersFactory.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

class Clock;
class PresenceLoader;

namespace PresenceLoaders
{

class OfflinePresenceStore;

namespace Factories
{

class Offline : public Factory
{
public:
    Offline( OfflinePresenceStore& sharedPresenceStore,
             Clock& clock );

    virtual PresenceLoader* makeLoader( const World::Coordinates& coordinates, bool receiveRequests );

private:
    OfflinePresenceStore& m_offlinePresenceStore;
    Clock& m_clock;
};

} // namespace Factories

} // namespace PresenceLoaders

} // namespace Agape

#endif // AGAPE_PRESENCE_LOADERS_FACTORIES_OFFLINE_H
