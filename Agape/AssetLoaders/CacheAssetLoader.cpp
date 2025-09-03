#include "AssetLoaders/Caches/AssetCache.h"
#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "Clocks/Clock.h"
#include "Encryptors/Utils/SecureIdentifier.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "World/WorldCoordinates.h"
#include "AssetLoader.h"
#include "CacheAssetLoader.h"
#include "String.h"

#include <string.h>

namespace Agape
{

namespace AssetLoaders
{

Cache::Cache( const World::Coordinates& coordinates,
              const String& name,
              Factory& assetLoaderBackingFactory,
              AssetCache& assetCache,
              bool encryptName ) :
  AssetLoader( coordinates, name ),
  m_assetLoaderBackingFactory( assetLoaderBackingFactory ),
  m_assetCache( assetCache ),
  m_encryptName( encryptName ),
  m_assetBackingLoader( nullptr ),
  m_currentCachedAsset( nullptr )
{
}

Cache::~Cache()
{
    if( m_currentCachedAsset )
    {
        m_currentCachedAsset->close();
    }
    delete( m_currentCachedAsset );

    if( m_assetBackingLoader )
    {
        m_assetBackingLoader->close();
    }
    delete( m_assetBackingLoader );
}

bool Cache::open()
{
    return open( modeRead, String() );
}

bool Cache::open( enum OpenMode openMode, const String& linkedItem )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "CacheAssetLoader: Opening " + m_name );
#endif

    if( m_currentCachedAsset || m_assetBackingLoader )
    {
        // Already opened!
#ifdef LOG_LOADERS
        LOG_DEBUG( "CacheAssetLoader: Already opened" );
#endif
        return false;
    }

    bool success( false );

    if( openMode == modeRead )
    {
        // Open existing cached or cache now and open if possible.
#ifdef LOG_LOADERS
        LOG_DEBUG( "CacheAssetLoader: Trying to open cached." );
#endif
        // Note: No need to pass linkedItem here, as that's only relevant for
        // tagging assets when creating them (openMode == modeWrite).
        m_currentCachedAsset = m_assetCache.tryOpen( nameOrHash( m_name ),
                                                     m_coordinates,
                                                     m_assetLoaderBackingFactory );
    }
    else if( openMode == modeWrite )
    {
        // Invalidate any existing cached asset.
        m_assetCache.invalidate( nameOrHash( m_name ), m_coordinates );
    }

    if( m_currentCachedAsset )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "CacheAssetLoader: Successfully opened cached." );
#endif
        success = true;
    }
    else
    {
        // Opening for write, or, if reading, unable to cache.
        // Use backing loader.
#ifdef LOG_LOADERS
        LOG_DEBUG( "CacheAssetLoader: Opening with backing loader." );
#endif
        m_assetBackingLoader = m_assetLoaderBackingFactory.makeLoader( m_coordinates, m_name );
        if( m_assetBackingLoader->open( openMode, linkedItem ) )
        {
            success = true;
        }
        else
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "CacheAssetLoader: Failed to open with backing loader." );
#endif
            delete( m_assetBackingLoader );
            m_assetBackingLoader = nullptr;
        }
    }

    return success;
}

int Cache::read( char* data, int offset, int len )
{
    if( m_currentCachedAsset )
    {
#ifdef LOG_LOADERS
        {
        LiteStream stream;
        stream << "CacheAssetLoader: Attempt to read from cache."
               << " Asset size: " << m_currentCachedAsset->size()
               << " Offset: " << offset
               << " Length: " << len;
        LOG_DEBUG( stream.str() );
        }
#endif
        int numRead( m_currentCachedAsset->read( data, offset, len ) );
#ifdef LOG_LOADERS
        {
        LiteStream stream;
        stream << "CacheAssetLoader: Read " << numRead << " from cache";
        LOG_DEBUG( stream.str() );
        }
#endif
        return numRead;
    }
    else if( m_assetBackingLoader )
    {
#ifdef LOG_LOADERS
        LiteStream stream;
        stream << "CacheAssetLoader: Attempt to read from backing loader."
               << " Asset size: " << m_assetBackingLoader->size()
               << " Offset: " << offset
               << " Length: " << len;
        LOG_DEBUG( stream.str() );
#endif
        if( ( offset < m_assetBackingLoader->size() ) &&
            ( ( offset + len ) <= m_assetBackingLoader->size() ) )
        {
            // Read from backing loader.
            return( m_assetBackingLoader->read( data, offset, len ) );
        }
    }

    LOG_DEBUG( "CacheAssetLoader: Bad read request" );
    return 0;
}

int Cache::write( const char* data, int offset, int len )
{
    if( m_assetBackingLoader )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "CacheAssetLoader: Write to backing loader" );
#endif
        return m_assetBackingLoader->write( data, offset, len );
    }

    return 0;
}

bool Cache::close()
{
    if( m_currentCachedAsset )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "CacheAssetLoader: Closing cached asset" );
#endif
        m_currentCachedAsset->close();
        delete( m_currentCachedAsset );
        m_currentCachedAsset = nullptr;
    }
    else if( m_assetBackingLoader )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "CacheAssetLoader: Closing backing loader" );
#endif
        m_assetBackingLoader->close();
        delete( m_assetBackingLoader );
        m_assetBackingLoader = nullptr;
    }

    return true;
}

int Cache::size()
{
    if( m_currentCachedAsset )
    {
        return m_currentCachedAsset->size();
    }
    else if( m_assetBackingLoader )
    {
        return m_assetBackingLoader->size();
    }

    return 0;
}

bool Cache::move( const String& newName )
{
    if( m_currentCachedAsset || m_assetBackingLoader )
    {
        // Can't move while opened.
#ifdef LOG_LOADERS
        LOG_DEBUG( "CacheAssetLoader: Error: Attempted to move while open" );
#endif
        return false;
    }

    bool success( true );

    m_assetBackingLoader = m_assetLoaderBackingFactory.makeLoader( m_coordinates, m_name );
    if( m_assetBackingLoader )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "CacheAssetLoader: Move: Invalidating move source and dest" );
#endif
        // Invalidate source and target caches, if any,
        m_assetCache.invalidate( nameOrHash( m_name ), m_coordinates );
        m_assetCache.invalidate( nameOrHash( newName ), m_coordinates );

        m_name = newName;

#ifdef LOG_LOADERS
        LOG_DEBUG( "CacheAssetLoader: Moving with backing loader" );
#endif
        success = m_assetBackingLoader->move( newName );
    }
    else
    {
        LOG_DEBUG( "CacheAssetLoader: Failed to open backing loader for move" );
    }

    return success;
}

bool Cache::erase()
{
    if( m_currentCachedAsset || m_assetBackingLoader )
    {
        // Can't erase while opened.
#ifdef LOG_LOADERS
        LOG_DEBUG( "CacheAssetLoader: Error: Attempted to erase while open" );
#endif
        return false;
    }

    bool success( true );

    m_assetBackingLoader = m_assetLoaderBackingFactory.makeLoader( m_coordinates, m_name );
    if( m_assetBackingLoader )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "CacheAssetLoader: Erase: Invalidating cached" );
#endif
        // Invalidate current cache entry, if any.
        m_assetCache.invalidate( nameOrHash( m_name ), m_coordinates );

#ifdef LOG_LOADERS
        LOG_DEBUG( "CacheAssetLoader: Erasing with backing loader" );
#endif
        success = m_assetBackingLoader->erase();
    }
    else
    {
        LOG_DEBUG( "CacheAssetLoader: Failed to open backing loader for erase" );
    }

    return success;
}

bool Cache::error()
{
    if( m_assetBackingLoader )
    {
        return m_assetBackingLoader->error();
    }

    return false;
}

void Cache::invalidateCached( bool all )
{
    if( all )
    {
        m_assetCache.invalidateAll();
    }
    else
    {
        m_assetCache.invalidate( nameOrHash( m_name ), m_coordinates );
    }
}

String Cache::nameOrHash( const String& name )
{
    if( m_encryptName )
    {
        String hash;
        String cipherText;
        Encryptors::Utils::SecureIdentifier::splitIdentifier( name, hash, cipherText );
        return hash;
    }

    return name;
}

} // namespace AssetLoaders

} // namespace Agape
