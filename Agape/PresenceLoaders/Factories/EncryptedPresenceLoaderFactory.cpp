#include "Encryptors/Factories/EncryptorsFactory.h"
#include "PresenceLoaders/PresenceLoader.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "EncryptedPresenceLoaderFactory.h"
#include "PresenceLoadersFactory.h"

namespace Agape
{

namespace PresenceLoaders
{

namespace Factories
{

Encrypted::Encrypted( Factory& backingLoaderFactory,
                      Metadata& worldMetadata,
                      Encryptors::Factory& encryptorFactory ) :
  m_backingLoaderFactory( backingLoaderFactory ),
  m_worldMetadata( worldMetadata ),
  m_encryptorFactory( encryptorFactory )
{
}

PresenceLoader* Encrypted::makeLoader( const World::Coordinates& coordinates, bool receiveRequests )
{
    return new PresenceLoaders::Encrypted( coordinates,
                                           receiveRequests,
                                           m_backingLoaderFactory,
                                           m_worldMetadata,
                                           m_encryptorFactory );
}

} // namespace Factories

} // namespace PresenceLoaders

} // namespace Agape
