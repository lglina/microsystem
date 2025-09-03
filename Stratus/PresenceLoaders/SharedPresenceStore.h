#ifndef AGAPE_PRESENCE_LOADERS_SHARED_PRESENCE_STORE_H
#define AGAPE_PRESENCE_LOADERS_SHARED_PRESENCE_STORE_H

#include "World/ScenePresence.h"
#include "Collections.h"
#include "String.h"

#include <mutex>

namespace Agape
{

namespace PresenceLoaders
{

class SharedPresenceStore
{
public:
    Map< String, World::ScenePresence > m_presences;
    Map< String, World::ScenePresence > m_presenceHistory;

    mutable std::mutex m_mutex;
};

} // namespace PresenceLoaders

} // namespace Agape

#endif // AGAPE_PRESENCE_LOADERS_SHARED_PRESENCE_STORE_H
