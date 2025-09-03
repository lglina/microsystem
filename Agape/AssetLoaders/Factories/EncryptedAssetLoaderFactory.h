#ifndef AGAPE_ASSET_LOADERS_FACTORIES_ENCRYPTED_H
#define AGAPE_ASSET_LOADERS_FACTORIES_ENCRYPTED_H

#include "AssetLoaders/AssetLoader.h"
#include "AssetLoadersFactory.h"
#include "String.h"

namespace Agape
{

namespace Encryptors
{
class BlockFactory;
class Factory;
}

namespace World
{
class Coordinates;
class Metadata;
} // namespace World

class Hash;

using namespace World;

namespace AssetLoaders
{

class Factory;

namespace Factories
{

class Encrypted : public Factory
{
public:
    Encrypted( AssetLoaders::Factory& backingLoaderFactory,
               Encryptors::BlockFactory& blockEncryptorFactory,
               Metadata& worldMetadata,
               const String& sharedAssetsWorldID, // Base-64 string.
               const char* sharedAssetsItemKey, // Fixed-length array.
               Encryptors::Factory* encryptorFactory,
               Hash* hash,
               bool encryptName );
    
    Encrypted( AssetLoaders::Factory& backingLoaderFactory,
               Encryptors::BlockFactory& blockEncryptorFactory,
               Metadata& worldMetadata );

    virtual AssetLoader* makeLoader( const World::Coordinates& coordinates, const String& name );

private:
    AssetLoaders::Factory& m_backingLoaderFactory;
    Encryptors::BlockFactory& m_blockEncryptorFactory;
    Metadata& m_worldMetadata;
    String m_sharedAssetsWorldID;
    const char* m_sharedAssetsItemKey;
    Encryptors::Factory* m_encryptorFactory;
    Hash* m_hash;
    bool m_encryptName;
};

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_FACTORIES_ENCRYPTED_H
