#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/AssetLoader.h"
#include "Clocks/Clock.h"
#include "Encryptors/Hash.h"
#include "Loggers/Logger.h"
#include "Utils/base64/base64.h"
#include "Utils/EscapeBase64.h"
#include "Utils/LiteStream.h"
#include "World/WorldCoordinates.h"
#include "AssetCache.h"
#include "Collections.h"
#include "KiamaFS.h"
#include "KiamaFSAssetCache.h"
#include "String.h"

namespace
{
    const int maxBlockSize( 256 );
} // Anonymous namespace

namespace Agape
{

using namespace World;

namespace AssetLoaders
{

namespace Caches
{

KiamaFSCachedAsset::KiamaFSCachedAsset( KiamaFS::File* file ) :
  m_file( file )
{
}

KiamaFSCachedAsset::~KiamaFSCachedAsset()
{
    delete( m_file );
}

int KiamaFSCachedAsset::read( char* data, int offset, int len )
{
    m_file->seek( offset ); // FIXME: Unnecessary for reads at monotonic offsets?
    return m_file->read( data, len );
}

int KiamaFSCachedAsset::size()
{
    return m_file->size();
}

void KiamaFSCachedAsset::close()
{
    // Nop.
}

KiamaFSAssetCache::KiamaFSAssetCache( int numAssets,
                                      int maxAssetSize,
                                      KiamaFS& kiamaFS,
                                      const String& extension,
                                      Hash& hash,
                                      Clock& clock ) :
  m_numAssets( numAssets ),
  m_maxAssetSize( maxAssetSize ),
  m_fs( kiamaFS ),
  m_extension( extension ),
  m_hash( hash ),
  m_clock( clock ),
  m_loaded( false )
{
}

CachedAsset* KiamaFSAssetCache::tryOpen( const String& assetName,
                                         const Coordinates& coordinates,
                                         AssetLoaders::Factory& backingLoaderFactory )
{
    if( !m_loaded ) load();

    KiamaFSCachedAsset* cachedAsset( nullptr );

#ifdef LOG_LOADERS
    LOG_DEBUG( "KiamaFSAssetCache: Looking for existing cached asset with name " + assetName );
#endif

    // Look for existing file.
    String filename( filenameForAsset( assetName, coordinates ) );
    Vector< _KiamaFSCachedAsset >::iterator it( m_cachedAssets.begin() );
    for( ; it != m_cachedAssets.end(); ++it )
    {
        if( it->m_name == filename )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "KiamaFSAssetCache: Found." );
#endif
            KiamaFS::File* file( m_fs.file( it->m_name ) );
            if( file->open( KiamaFS::File::readMode ) )
            {
                cachedAsset = new KiamaFSCachedAsset( file );
                break;
            }
            else
            {
#ifdef LOG_LOADERS
                LOG_DEBUG( "KiamaFSAssetCache: Found and failed to open." );
#endif
                delete( file );
            }
        }
    }

    if( !cachedAsset )
    {
        // No existing cached asset. Try to cache now.
#ifdef LOG_LOADERS
        LOG_DEBUG( "KiamaFSAssetCache: Existing cached asset not found. Caching now." );
#endif
        cachedAsset = tryCache( assetName,
                                coordinates,
                                backingLoaderFactory );
    }

    return cachedAsset;
}

void KiamaFSAssetCache::invalidate( const String& assetName,
                                    const Coordinates& coordinates )
{
    if( !m_loaded ) load();

#ifdef LOG_LOADERS
    LOG_DEBUG( "KiamaFSAssetCache: Invalidating " + assetName );
#endif

    String filename( filenameForAsset( assetName, coordinates ) );
    Vector< _KiamaFSCachedAsset >::iterator it( m_cachedAssets.begin() );
    for( ; it != m_cachedAssets.end(); ++it )
    {
        if( it->m_name == filename )
        {
            KiamaFS::File* file( m_fs.file( it->m_name ) );
            file->erase();
            delete( file );
            break;
        }
    }
}

void KiamaFSAssetCache::invalidateAll()
{
    if( !m_loaded ) load();

#ifdef LOG_LOADERS
    LOG_DEBUG( "KiamaFSAssetCache: Invalidating all" );
#endif
    Vector< _KiamaFSCachedAsset >::iterator it( m_cachedAssets.begin() );
    for( ; it != m_cachedAssets.end(); ++it )
    {
        KiamaFS::File* file( m_fs.file( it->m_name ) );
        file->erase();
        delete( file );
    }
}

void KiamaFSAssetCache::load()
{
    const Map< String, KiamaFS::IndexEntry >& index( m_fs.getIndex() );
    Map< String, KiamaFS::IndexEntry >::const_iterator it( index.begin() );
    for( ; it != index.end(); ++it )
    {
        if( ( it->first[0] == '$' ) &&
            ( it->first.find( m_extension, it->first.length() - m_extension.length() ) != String::npos ) )
        {
#ifdef LOG_LOADERS
            LiteStream stream;
            stream << "KiamaFSAssetCache: Found existing cached asset "
                   << it->first
                   << " with size "
                   << it->second.m_size ;
            LOG_DEBUG( stream.str() );
#endif
            struct _KiamaFSCachedAsset _cachedAsset;
            _cachedAsset.m_name = it->first;
            _cachedAsset.m_lastAccessed = m_clock.epochS();
            m_cachedAssets.push_back( _cachedAsset );
        }
    }

    m_loaded = true;
}

KiamaFSCachedAsset* KiamaFSAssetCache::tryCache( const String& assetName,
                                                 const Coordinates& coordinates,
                                                 AssetLoaders::Factory& backingLoaderFactory )
{
    KiamaFSCachedAsset* cachedAsset( nullptr );

    while( m_cachedAssets.size() >= m_numAssets )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "KiamaFSAssetCache: Cache full. Evicting oldest." );
#endif
        if( !evictOldest() ) return nullptr; // Uh oh!
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "KiamaFSAssetCache: Opening asset with backing loader to cache" );
#endif
    AssetLoader* assetBackingLoader( backingLoaderFactory.makeLoader( coordinates, assetName ) );
    if( assetBackingLoader->open() && ( assetBackingLoader->size() <= m_maxAssetSize ) )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "KiamaFSAssetCache: Opening KiamaFS file to write" );
#endif
        String filename( filenameForAsset( assetName, coordinates ) );
        KiamaFS::File* writeFile( m_fs.file( filename ) );
        if( writeFile->open( KiamaFS::File::writeMode ) )
        {
            char buffer[maxBlockSize];
            int offset( 0 );
            int assetSize( assetBackingLoader->size() );
            bool error( false );
            while( ( offset < assetSize ) && !error )
            {
                int numRemain( assetSize - offset );
                int numToRead( ( numRemain > maxBlockSize ) ? maxBlockSize : numRemain );
                int numRead( assetBackingLoader->read( buffer, offset, numToRead ) );
                int numWritten( writeFile->write( buffer, numRead ) );

                // N.B. Assumes filesystem can write the whole buffer in one go...
                if( ( numRead > 0 ) &&
                    ( numWritten == numRead ) &&
                    !assetBackingLoader->error() )
                {
                    offset += numRead;
                }
                else
                {
                    error = true;
                }
            }

            if( !error )
            {
#ifdef LOG_LOADERS
                LOG_DEBUG( "KiamaFSAssetCache: Cached data written successfully. Committing and opening for reading." );
#endif
                writeFile->commit();
                KiamaFS::File* readFile( m_fs.file( filename ) );
                if( readFile->open( KiamaFS::File::readMode ) )
                {
#ifdef LOG_LOADERS
                    LOG_DEBUG( "KiamaFSAssetCache: Committed and opened successfully." );
#endif
                    // Return new cached asset to original caller.
                    cachedAsset = new KiamaFSCachedAsset( readFile );

                    // Store filename in our list of extant cached assets.
                    struct _KiamaFSCachedAsset _cachedAsset;
                    _cachedAsset.m_name = filename;
                    _cachedAsset.m_lastAccessed = m_clock.epochS();
                    m_cachedAssets.push_back( _cachedAsset );
                }
                else
                {
#ifdef LOG_LOADERS
                    LOG_DEBUG( "KiamaFSAssetCache: Failed to open cached asset" );
#endif
                    delete( readFile );
                }
            }
        }

        delete( writeFile );
    }

    assetBackingLoader->close();
    delete( assetBackingLoader );

    return cachedAsset;
}

bool KiamaFSAssetCache::evictOldest()
{
    long long oldestTime( m_clock.epochS() );
    int oldestIdx( -1 );
    Vector< _KiamaFSCachedAsset >::iterator it( m_cachedAssets.begin() );
    Vector< _KiamaFSCachedAsset >::iterator oldestIt( m_cachedAssets.end() );
    for( ; it != m_cachedAssets.end(); ++it )
    {
        if( it->m_lastAccessed <= oldestTime )
        {
            oldestIt = it;
            oldestTime = it->m_lastAccessed;
        }
    }

    if( oldestIt != m_cachedAssets.end() )
    {
#ifdef LOG_LOADERS
        LiteStream stream;
        stream << "KiamaFSAssetCache: Evicting "
               << oldestIt->m_name
               << " with last accessed time "
               << oldestIt->m_lastAccessed;
        LOG_DEBUG( stream.str() );
#endif
        KiamaFS::File* file( m_fs.file( oldestIt->m_name ) );
        file->erase();
        delete( file );
        m_cachedAssets.erase( oldestIt );
        return true;
    }

    return false;
}

String KiamaFSAssetCache::filenameForAsset( const String& assetName,
                                            const Coordinates& coordinates )
{
    // Create filename in format:
    // $aaaaaaaa_bbbbbbbb.ext
    // aaaaaaaa = First eight characters of world ID.
    // bbbbbbbb = First eight characters of Base64 of hash of assetName.
    // Base64 to have substitutions '_' => '/', '-' => '+'.

    String worldPart( escapeBase64( coordinates.m_worldID.substr( 0, 8 ) ) );

    m_hash.reset();
    m_hash.update( assetName.c_str(), assetName.length() );
    char digest[m_hash.digestSize()];
    m_hash.finalise( digest );

    String encodedDigest( Base64encode_len( m_hash.digestSize() ), '\0' );
    Base64encode( &encodedDigest[0],
                  &digest[0],
                  m_hash.digestSize() );
    encodedDigest.resize( encodedDigest.length() - 1 );

    String hashPart( escapeBase64( encodedDigest.substr( 0, 8 ) ) );

    return( "$" + worldPart + "_" + hashPart + "." + m_extension );
}

} // namespace Caches

} // namespace AssetLoaders

} // namespace Agape
