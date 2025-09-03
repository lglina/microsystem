#ifndef AGAPE_ASSET_CACHE_H
#define AGAPE_ASSET_CACHE_H

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

using namespace World;

class String;

namespace AssetLoaders
{

class Factory;

class CachedAsset
{
public:
    virtual ~CachedAsset() {};

    virtual int read( char* data, int offset, int len ) = 0;
    virtual int size() = 0;
    virtual void close() = 0;
};

class AssetCache
{
public:
    virtual ~AssetCache() {};

    virtual CachedAsset* tryOpen( const String& assetName,
                                  const Coordinates& coordinates,
                                  AssetLoaders::Factory& backingLoaderFactory ) = 0;

    virtual void invalidate( const String& assetName,
                             const Coordinates& coordinates ) = 0;
    virtual void invalidateAll() = 0;
};

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_CACHE_H
