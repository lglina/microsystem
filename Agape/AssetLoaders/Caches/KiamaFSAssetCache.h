#ifndef AGAPE_ASSET_CACHES_KIAMA_FS_H
#define AGAPE_ASSET_CACHES_KIAMA_FS_H

#include "AssetCache.h"
#include "Collections.h"
#include "KiamaFS.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

class Clock;
class Hash;
class String;

using namespace World;

namespace AssetLoaders
{

class Factory;

namespace Caches
{

class KiamaFSCachedAsset : public CachedAsset
{
public:
    KiamaFSCachedAsset( KiamaFS::File* file );
    ~KiamaFSCachedAsset();

    virtual int read( char* data, int offset, int len );
    virtual int size();
    virtual void close();

private:
    KiamaFS::File* m_file;
};

class KiamaFSAssetCache : public AssetCache
{
public:
    KiamaFSAssetCache( int numAssets,
                       int maxAssetSize,
                       KiamaFS& kiamaFS,
                       const String& extension,
                       Hash& hash,
                       Clock& clock );

    virtual CachedAsset* tryOpen( const String& assetName,
                                  const Coordinates& coordinates,
                                  AssetLoaders::Factory& backingLoaderFactory );

    virtual void invalidate( const String& assetName,
                             const Coordinates& coordinates );
    virtual void invalidateAll();

private:
    struct _KiamaFSCachedAsset
    {
        String m_name;
        long long m_lastAccessed;
    };

    void load();

    KiamaFSCachedAsset* tryCache( const String& assetName,
                                  const Coordinates& coordinates,
                                  AssetLoaders::Factory& backingLoaderFactory );
    bool evictOldest();

    String filenameForAsset( const String& assetName,
                             const Coordinates& coordinates );

    int m_numAssets;
    int m_maxAssetSize;
    KiamaFS& m_fs;
    String m_extension;
    Hash& m_hash;
    Clock& m_clock;

    bool m_loaded;
    Vector< _KiamaFSCachedAsset > m_cachedAssets;
};

} // namespace Caches

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_CACHES_KIAMA_FS_H
