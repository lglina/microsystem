#ifndef AGAPE_PRESENCE_LOADERS_OFFLINE_H
#define AGAPE_PRESENCE_LOADERS_OFFLINE_H

#include "World/ScenePresence.h"
#include "Collections.h"
#include "PresenceLoader.h"
#include "PresenceRequest.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

class Clock;

namespace PresenceLoaders
{

class OfflinePresenceStore;

class Offline : public PresenceLoader
{
public:
    Offline( const World::Coordinates& coordinates,
             OfflinePresenceStore& offlinePresenceStore,
             Clock& clock );

    virtual bool load( Vector< World::ScenePresence >& scenePresences );

    virtual bool loadWorld( Vector< ScenePresence >& worldPresences );

    virtual bool request( const Vector< PresenceRequest >& requests );
    virtual Vector< PresenceRequest > getUpdates();

private:
    void _arrive( const PresenceRequest& request );
    void _depart( const PresenceRequest& request );
    void _move( const PresenceRequest& request );

    OfflinePresenceStore& m_offlinePresenceStore;
    Clock& m_clock;
};

} // namespace PresenceLoaders

} // namespace Agape

#endif // AGAPE_PRESENCE_LOADERS_OFFLINE_H
