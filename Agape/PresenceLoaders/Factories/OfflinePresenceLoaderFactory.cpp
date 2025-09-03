#include "Clocks/Clock.h"
#include "PresenceLoaders/OfflinePresenceLoader.h"
#include "PresenceLoaders/OfflinePresenceStore.h"
#include "OfflinePresenceLoaderFactory.h"

namespace Agape
{

namespace PresenceLoaders
{

namespace Factories
{

Offline::Offline( OfflinePresenceStore& offlinePresenceStore,
                  Clock& clock ) :
  m_offlinePresenceStore( offlinePresenceStore ),
  m_clock( clock )
{
}

PresenceLoader* Offline::makeLoader( const World::Coordinates& coordinates, bool )
{
    return new PresenceLoaders::Offline( coordinates,
                                         m_offlinePresenceStore,
                                         m_clock );
}

} // namespace Factories

} // namespace PresenceLoaders

} // namespace Agape
