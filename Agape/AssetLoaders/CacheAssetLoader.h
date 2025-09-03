#ifndef AGAPE_ASSET_LOADERS_CACHE_H
#define AGAPE_ASSET_LOADERS_CACHE_H

#include "AssetLoader.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

class String;

namespace AssetLoaders
{

class AssetCache;
class CachedAsset;
class Factory;

class Cache : public AssetLoader
{
public:
    Cache( const World::Coordinates& coordinates,
           const String& name,
           Factory& assetLoaderBackingFactory,
           AssetCache& assetCache,
           bool encryptName );
    ~Cache();

    virtual bool open();
    virtual bool open( enum OpenMode openMode, const String& linkedItem );
    virtual int read( char* data, int offset, int len );
    virtual int write( const char* data, int offset, int len );
    virtual bool close();
    virtual int size();

    virtual bool move( const String& newName );
    virtual bool erase();

    virtual bool error();

    virtual void invalidateCached( bool all = false );

private:
    String nameOrHash( const String& name );

    Factory& m_assetLoaderBackingFactory;
    AssetCache& m_assetCache;
    bool m_encryptName;

    AssetLoader* m_assetBackingLoader;
    CachedAsset* m_currentCachedAsset;
};

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_CACHE_H
