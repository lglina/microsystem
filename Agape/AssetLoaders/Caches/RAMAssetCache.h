#ifndef AGAPE_ASSET_CACHES_RAM_H
#define AGAPE_ASSET_CACHES_RAM_H

#include "AssetCache.h"

#ifdef __XC32
#define CACHE_NAME_MAX_LENGTH 32
#else
#define CACHE_NAME_MAX_LENGTH 64 // Sufficient to hold hashes of assets on Stratus
#endif

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

class Clock;
class String;

using namespace World;

namespace AssetLoaders
{

class Factory;

namespace Caches
{

class RAMCachedAsset : public CachedAsset
{
public:
    RAMCachedAsset( int size, const char* data );

    virtual int read( char* data, int offset, int len );
    virtual int size();
    virtual void close();

private:
    int m_size;
    const char* m_data;
};

class RAMAssetCache : public AssetCache
{
public:
    RAMAssetCache( int numAssets,
                   int maxAssetSize,
                   Clock& clock );
    ~RAMAssetCache();

    virtual CachedAsset* tryOpen( const String& assetName,
                                  const Coordinates& coordinates,
                                  AssetLoaders::Factory& backingLoaderFactory );

    virtual void invalidate( const String& assetName,
                             const Coordinates& coordinates );
    virtual void invalidateAll();

private:
    struct _RAMCachedAsset
    {
        char m_name[CACHE_NAME_MAX_LENGTH];
        long long m_lastAccessed;
        int m_size;
        char* m_data;
    };

    RAMCachedAsset* tryCache( const String& assetName,
                              const Coordinates& coordinates,
                              AssetLoaders::Factory& backingLoaderFactory );
    _RAMCachedAsset* findFree();
    _RAMCachedAsset* evictOldest();

    int m_numAssets;
    int m_maxAssetSize;
    Clock& m_clock;
    _RAMCachedAsset* m_assets;
    char* m_slab;
};

} // namespace Caches

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_CACHES_RAM_H
