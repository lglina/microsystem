#ifndef AGAPE_PRESENCE_LOADERS_OFFLINE_PRESENCE_STORE_H
#define AGAPE_PRESENCE_LOADERS_OFFLINE_PRESENCE_STORE_H

#include "World/ScenePresence.h"
#include "Collections.h"
#include "String.h"

namespace Agape
{

namespace PresenceLoaders
{

class OfflinePresenceStore
{
public:
    Map< String, World::ScenePresence > m_presences;
    Map< String, World::ScenePresence > m_presenceHistory;
};

} // namespace PresenceLoaders

} // namespace Agape

#endif // AGAPE_PRESENCE_LOADERS_OFFLINE_PRESENCE_STORE_H
