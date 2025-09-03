#ifndef AGAPE_PRESENCE_LOADER_H
#define AGAPE_PRESENCE_LOADER_H

#include "World/WorldCoordinates.h"
#include "World/ScenePresence.h"
#include "Collections.h"
#include "PresenceRequest.h"

using namespace Agape::World;

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

class PresenceLoader
{
public:
    static bool noReceiveRequests;

    PresenceLoader( const World::Coordinates& coordinates );

    virtual ~PresenceLoader();

    virtual bool load( Vector< ScenePresence >& scenePresences ) = 0;

    virtual bool loadWorld( Vector< ScenePresence >& worldPresences ) = 0;

    virtual bool request( const Vector< PresenceRequest >& requests ) = 0;
    virtual Vector< PresenceRequest > getUpdates() = 0; // FIXME: Should this return a reference?

    virtual bool overflowed() { return false; }; // If implemented, should clear flag on retrieve if set.

protected:
    Coordinates m_coordinates;
};

} // namespace Agape

#endif // AGAPE_PRESENCE_LOADER_H
