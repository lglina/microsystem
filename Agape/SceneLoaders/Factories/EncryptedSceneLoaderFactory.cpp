#include "Encryptors/Factories/EncryptorsFactory.h"
#include "Encryptors/Hash.h"
#include "SceneLoaders/EncryptedSceneLoader.h"
#include "SceneLoaders/SceneLoader.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "EncryptedSceneLoaderFactory.h"
#include "String.h"

namespace Agape
{

using namespace World;

namespace SceneLoaders
{

namespace Factories
{

Encrypted::Encrypted( Factory& backingLoaderFactory,
                      Metadata& worldMetadata,
                      Encryptors::Factory& encryptorFactory,
                      Hash& hash ) :
  m_backingLoaderFactory( backingLoaderFactory ),
  m_worldMetadata( worldMetadata ),
  m_encryptorFactory( encryptorFactory ),
  m_hash( hash )
{
}

SceneLoader* Encrypted::makeLoader( const Coordinates& coordinates, bool receiveRequests )
{
    return new SceneLoaders::Encrypted( coordinates,
                                        m_backingLoaderFactory,
                                        m_worldMetadata,
                                        m_encryptorFactory,
                                        m_hash );
}

} // namespace Factories

} // namespace SceneLoaders

} // namespace Agape
