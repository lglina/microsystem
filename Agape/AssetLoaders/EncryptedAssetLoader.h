#ifndef AGAPE_ASSET_LOADERS_ENCRYPTED_H
#define AGAPE_ASSET_LOADERS_ENCRYPTED_H

#include "AssetLoader.h"
#include "String.h"

namespace Agape
{

namespace Encryptors
{
class BlockFactory;
class Factory;
} // namespace Encryptors

namespace World
{
class Coordinates;
class Metadata;
} // namespace World

class BlockEncryptor;
class Encryptor;
class Hash;

using namespace World;

namespace AssetLoaders
{

class Factory;

class Encrypted : public AssetLoader
{
public:
    Encrypted( const World::Coordinates& coordinates,
               const String& name,
               Factory& backingLoaderFactory,
               Encryptors::BlockFactory& blockEncryptorFactory,
               Metadata& worldMetadata,
               const String& sharedAssetsWorldID, // Base-64 string.
               const char* sharedAssetsItemKey, // Fixed-length array.
               Encryptors::Factory* encryptorFactory,
               Hash* hash,
               bool encryptName );
    ~Encrypted();

    virtual bool open();
    virtual bool open( enum OpenMode openMode, const String& linkedItem );
    virtual int read( char* data, int offset, int len );
    virtual int write( const char* data, int offset, int len );
    virtual bool close();
    virtual int size();

    virtual bool move( const String& newName );
    virtual bool erase();

    virtual bool error();

private:
    Factory& m_backingLoaderFactory;
    Metadata& m_worldMetadata;
    String m_sharedAssetsWorldID;
    const char* m_sharedAssetsItemKey;
    Hash* m_hash;
    bool m_encryptName;

    BlockEncryptor* m_blockEncryptor;
    Encryptor* m_encryptor;

    AssetLoader* m_backingLoader;

    String encryptedAssetName( const char* itemKey,
                               const String& assetNamePlain );

    bool tryOpen( enum OpenMode openMode,
                  const String& linkedItem,
                  const String& worldID,
                  const char* itemKey );
};

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_ENCRYPTED_H
