#ifndef AGAPE_PRESENCE_LOADERS_SHARED_H
#define AGAPE_PRESENCE_LOADERS_SHARED_H

#include "World/ScenePresence.h"
#include "Collections.h"
#include "PresenceLoader.h"
#include "PresenceRequest.h"

namespace Agape
{

namespace Stratus
{
class Authenticator;
} // namespace Stratus

namespace World
{
class Coordinates;
} // namespace World

class Clock;

using namespace Stratus;

namespace PresenceLoaders
{

class SharedPresenceStore;

class Shared : public PresenceLoader
{
public:
    Shared( const World::Coordinates& coordinates,
            SharedPresenceStore& sharedPresenceStore,
            Clock& clock,
            Authenticator& authenticator );

    virtual bool load( Vector< World::ScenePresence >& scenePresences );

    virtual bool loadWorld( Vector< ScenePresence >& worldPresences );

    virtual bool request( const Vector< PresenceRequest >& requests );
    virtual Vector< PresenceRequest > getUpdates();

private:
    void _arrive( const PresenceRequest& request );
    void _depart( const PresenceRequest& request );
    void _move( const PresenceRequest& request );

    SharedPresenceStore& m_sharedPresenceStore;
    Clock& m_clock;
    Authenticator& m_authenticator;
};

} // namespace PresenceLoaders

} // namespace Agape

#endif // AGAPE_PRESENCE_LOADERS_SHARED_H
