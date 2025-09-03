#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "Clocks/Clock.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "World/WorldCoordinates.h"
#include "RAMAssetCache.h"
#include "String.h"

#include <string.h>

namespace Agape
{

using namespace World;

namespace AssetLoaders
{

namespace Caches
{

RAMCachedAsset::RAMCachedAsset( int size, const char* data ) :
  m_size( size ),
  m_data( data )
{
}

int RAMCachedAsset::read( char* data, int offset, int len )
{
    if( ( offset < m_size ) &&
        ( ( offset + len ) <= m_size ) )
    {
        ::memcpy( data, m_data + offset, len );
        return len;
    }

    return 0;
}

int RAMCachedAsset::size()
{
    return m_size;
}

void RAMCachedAsset::close()
{
    // Nop.
}

RAMAssetCache::RAMAssetCache( int numAssets,
                              int maxAssetSize,
                              Clock& clock ) :
  m_numAssets( numAssets ),
  m_maxAssetSize( maxAssetSize ),
  m_clock( clock )
{
    m_assets = new _RAMCachedAsset[numAssets];
    m_slab = new char[numAssets * maxAssetSize];
    ::memset( m_assets, '\0', sizeof( CachedAsset ) * numAssets );

    for( int i = 0; i < numAssets; ++i )
    {
        m_assets[i].m_data = m_slab + ( m_maxAssetSize * i );
    }
}

RAMAssetCache::~RAMAssetCache()
{
    delete[]( m_assets );
    delete[]( m_slab );
}

CachedAsset* RAMAssetCache::tryOpen( const String& assetName,
                                     const Coordinates& coordinates,
                                     AssetLoaders::Factory& backingLoaderFactory )
{
    RAMCachedAsset* cachedAsset( nullptr );

#ifdef LOG_LOADERS
    LOG_DEBUG( "RAMAssetCache: Looking for cached with name " + assetName );
#endif

    if( assetName.length() <= CACHE_NAME_MAX_LENGTH )
    {
        // Look for already cached.
        bool found( false );
        for( int i = 0; !found && ( i < m_numAssets ); ++i )
        {
            _RAMCachedAsset& _cachedAsset( m_assets[i] );
            if( ::strncmp( _cachedAsset.m_name, assetName.c_str(), assetName.length() ) == 0 )
            {
                // Found.
#ifdef LOG_LOADERS
                LOG_DEBUG( "RAMAssetCache: Found." );
#endif
                _cachedAsset.m_lastAccessed = m_clock.epochS();
                cachedAsset = new RAMCachedAsset( _cachedAsset.m_size,
                                                  _cachedAsset.m_data );
                found = true;
                break;
            }
        }

        if( !found )
        {
            // Not found. Try to cache now.
#ifdef LOG_LOADERS
            LOG_DEBUG( "CacheAssetLoader: Not found. Caching." );
#endif
            cachedAsset = tryCache( assetName,
                                    coordinates,
                                    backingLoaderFactory );
        }
    }
#ifdef LOG_LOADERS
    else
    {
        LOG_DEBUG( "CacheAssetLoader: Asset ineligible for caching - filename too long." );
    }
#endif

    return cachedAsset; // If nullptr, caller will load directly from backing loader.
}

void RAMAssetCache::invalidate( const String& assetName,
                                const Coordinates& coordinates )
{
    if( assetName.length() <= CACHE_NAME_MAX_LENGTH )
    {
        for( int i = 0; i < m_numAssets; ++i )
        {
            _RAMCachedAsset& _cachedAsset( m_assets[i] );
            if( ::strncmp( _cachedAsset.m_name, assetName.c_str(), assetName.length() ) == 0 )
            {
#ifdef LOG_LOADERS
                LOG_DEBUG( "RAMAssetCache: Invalidating " + assetName );
#endif
                _cachedAsset.m_name[0] = '\0';
                return;
            }
        }
    }
}

void RAMAssetCache::invalidateAll()
{
    for( int i = 0; i < m_numAssets; ++i )
    {
        _RAMCachedAsset& _cachedAsset( m_assets[i] );
        _cachedAsset.m_name[0] = '\0';
    }
#ifdef LOG_LOADERS
    LOG_DEBUG( "RAMAssetCache: Evicted all assets in cache" );
#endif
}

RAMCachedAsset* RAMAssetCache::tryCache( const String& assetName,
                                         const Coordinates& coordinates,
                                         AssetLoaders::Factory& backingLoaderFactory )
{
    // Pre-requisite: assetName is <= 32 chars.
    _RAMCachedAsset* _cachedAsset( findFree() );
    RAMCachedAsset* cachedAsset( nullptr );

    if( _cachedAsset )
    {
        AssetLoader* assetBackingLoader( backingLoaderFactory.makeLoader( coordinates, assetName ) );
        if( assetBackingLoader->open() && ( assetBackingLoader->size() <= m_maxAssetSize ) )
        {
    #ifdef LOG_LOADERS
            LOG_DEBUG( "RAMAssetCache: Caching from backing loader" );
    #endif
            ::strncpy( _cachedAsset->m_name, assetName.c_str(), CACHE_NAME_MAX_LENGTH );
            _cachedAsset->m_lastAccessed = m_clock.epochS();
            _cachedAsset->m_size = assetBackingLoader->size();

            int numToRead( _cachedAsset->m_size < m_maxAssetSize ? _cachedAsset->m_size : m_maxAssetSize );
            bool success = ( assetBackingLoader->readAll( _cachedAsset->m_data, 0, numToRead ) == numToRead );
            if( success )
            {
#ifdef LOG_LOADERS
                LOG_DEBUG( "RAMAssetCache: Successfully cached" );
#endif
                cachedAsset = new RAMCachedAsset( _cachedAsset->m_size,
                                                  _cachedAsset->m_data );
            }
#ifdef LOG_LOADERS
            else
            {
                LOG_DEBUG( "RAMAssetCache: Failed to read from backing loader" );
            }
#endif
        }
#ifdef LOG_LOADERS
        else
        {
            LOG_DEBUG( "RAMAssetCache: Asset ineligible for caching - asset too large." );
        }
#endif

        delete( assetBackingLoader );
    }
#ifdef LOG_LOADERS
    else
    {
        LOG_DEBUG( "RAMAssetCache: Unable to cache - free entry not found" );
    }
#endif

    return cachedAsset;
}

RAMAssetCache::_RAMCachedAsset* RAMAssetCache::findFree()
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "RAMAssetCache: Looking for free cache entry" );
#endif
    for( int i = 0; i < m_numAssets; ++i )
    {
        _RAMCachedAsset& _cachedAsset( m_assets[i] );
        if( _cachedAsset.m_name[0] == '\0' )
        {
            // Free slot.
#ifdef LOG_LOADERS
            LOG_DEBUG( "RAMAssetCache: Found free cache entry" );
#endif
            return &_cachedAsset;
        }
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "RAMAssetCache: No free cache entries. Evicting oldest." );
#endif
    return evictOldest();
}

RAMAssetCache::_RAMCachedAsset* RAMAssetCache::evictOldest()
{
    long long oldestTime( m_clock.epochS() );
    int oldestIdx( -1 );
    for( int i = 0; i < m_numAssets; ++i )
    {
        _RAMCachedAsset& _cachedAsset( m_assets[i] );
        if( ( _cachedAsset.m_name[0] != '\0' ) &&
            ( _cachedAsset.m_lastAccessed <= oldestTime ) )
        {
            oldestTime = _cachedAsset.m_lastAccessed;
            oldestIdx = i;
        }
    }

    if( oldestIdx != -1 )
    {
        _RAMCachedAsset& _cachedAsset( m_assets[oldestIdx] );
#ifdef LOG_LOADERS
        LiteStream stream;
        stream << "RAMAssetCache: Evicted cache entry for asset " << _cachedAsset.m_name << " with time " << _cachedAsset.m_lastAccessed;
        LOG_DEBUG( stream.str() );
#endif
        _cachedAsset.m_name[0] = '\0';
        return &_cachedAsset;
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "RAMAssetCache: Unable to evict" );
#endif
    return nullptr;
}

} // namespace Caches

} // namespace AssetLoaders

} // namespace Agape
