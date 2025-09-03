#include "Clocks/Clock.h"
#include "PresenceLoaders/SharedPresenceLoader.h"
#include "PresenceLoaders/SharedPresenceStore.h"
#include "Authenticator.h"
#include "SharedPresenceLoaderFactory.h"

using namespace Agape::Stratus;

namespace Agape
{

namespace PresenceLoaders
{

namespace Factories
{

Shared::Shared( SharedPresenceStore& sharedPresenceStore,
                Clock& clock,
                Authenticator& authenticator ) :
  m_sharedPresenceStore( sharedPresenceStore ),
  m_clock( clock ),
  m_authenticator( authenticator )
{
}

PresenceLoader* Shared::makeLoader( const World::Coordinates& coordinates, bool )
{
    return new PresenceLoaders::Shared( coordinates,
                                        m_sharedPresenceStore,
                                        m_clock,
                                        m_authenticator );
}

} // namespace Factories

} // namespace PresenceLoaders

} // namespace Agape
