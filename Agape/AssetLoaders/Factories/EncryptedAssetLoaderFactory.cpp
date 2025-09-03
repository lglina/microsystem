#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/EncryptedAssetLoader.h"
#include "Encryptors/Factories/BlockEncryptorsFactory.h"
#include "Encryptors/Factories/EncryptorsFactory.h"
#include "Encryptors/Hash.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "EncryptedAssetLoaderFactory.h"
#include "String.h"

using namespace Agape::World;

namespace Agape
{

namespace AssetLoaders
{

namespace Factories
{

Encrypted::Encrypted( AssetLoaders::Factory& backingLoaderFactory,
                      Encryptors::BlockFactory& blockEncryptorFactory,
                      Metadata& worldMetadata,
                      const String& sharedAssetsWorldID,
                      const char* sharedAssetsItemKey,
                      Encryptors::Factory* encryptorFactory,
                      Hash* hash,
                      bool encryptName ) :
  m_backingLoaderFactory( backingLoaderFactory ),
  m_blockEncryptorFactory( blockEncryptorFactory ),
  m_worldMetadata( worldMetadata ),
  m_sharedAssetsWorldID( sharedAssetsWorldID ),
  m_sharedAssetsItemKey( sharedAssetsItemKey ),
  m_encryptorFactory( encryptorFactory ),
  m_hash( hash ),
  m_encryptName( encryptName )
{
}
    
Encrypted::Encrypted( AssetLoaders::Factory& backingLoaderFactory,
                      Encryptors::BlockFactory& blockEncryptorFactory,
                      Metadata& worldMetadata ) :
  m_backingLoaderFactory( backingLoaderFactory ),
  m_blockEncryptorFactory( blockEncryptorFactory ),
  m_worldMetadata( worldMetadata ),
  m_sharedAssetsItemKey( nullptr ),
  m_encryptorFactory( nullptr ),
  m_hash( nullptr ),
  m_encryptName( false )
{
}

AssetLoader* Encrypted::makeLoader( const World::Coordinates& coordinates, const String& name )
{
    return( new AssetLoaders::Encrypted( coordinates,
                                         name,
                                         m_backingLoaderFactory,
                                         m_blockEncryptorFactory,
                                         m_worldMetadata,
                                         m_sharedAssetsWorldID,
                                         m_sharedAssetsItemKey,
                                         m_encryptorFactory,
                                         m_hash,
                                         m_encryptName ) );
}

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape
