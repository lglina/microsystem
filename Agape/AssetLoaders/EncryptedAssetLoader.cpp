#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "Encryptors/Factories/BlockEncryptorsFactory.h"
#include "Encryptors/Factories/EncryptorsFactory.h"
#include "Encryptors/Utils/SecureIdentifier.h"
#include "Encryptors/BlockEncryptor.h"
#include "Encryptors/Encryptor.h"
#include "Encryptors/Hash.h"
#include "Loggers/Logger.h"
#include "Utils/base64/base64.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "AssetLoader.h"
#include "EncryptedAssetLoader.h"
#include "String.h"
#include "StringConstants.h"

using namespace Agape::World;

namespace Agape
{

namespace AssetLoaders
{

Encrypted::Encrypted( const World::Coordinates& coordinates,
                      const String& name,
                      AssetLoaders::Factory& backingLoaderFactory,
                      Encryptors::BlockFactory& blockEncryptorFactory,
                      Metadata& worldMetadata,
                      const String& sharedAssetsWorldID,
                      const char* sharedAssetsItemKey,
                      Encryptors::Factory* encryptorFactory,
                      Hash* hash,
                      bool encryptName ) :
  AssetLoader( coordinates, name ),
  m_backingLoaderFactory( backingLoaderFactory ),
  m_worldMetadata( worldMetadata ),
  m_sharedAssetsWorldID( sharedAssetsWorldID ),
  m_sharedAssetsItemKey( sharedAssetsItemKey ),
  m_hash( hash ),
  m_encryptName( encryptName ),
  m_backingLoader( nullptr )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "EncryptedAssetLoader: Constructing" );
#endif
    m_blockEncryptor = blockEncryptorFactory.makeEncryptor();
    if( encryptorFactory )
    {
        m_encryptor = encryptorFactory->makeEncryptor();
    }
    else
    {
        m_encryptor = nullptr;
    }
}

Encrypted::~Encrypted()
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "EncryptedAssetLoader: Destructing" );
#endif
    delete( m_blockEncryptor );
    delete( m_encryptor );
    delete( m_backingLoader );
}

bool Encrypted::open()
{
    return open( modeRead, String() );
}

bool Encrypted::open( enum OpenMode openMode, const String& linkedItem )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "EncryptedAssetLoader: Opening" );
#endif
    bool success( false );

    if( openMode == modeRead )
    {
        if( ( m_name == _message ) ||
            ( m_name == _World ) ||
            ( m_name.rfind( _scene_, 0 ) == 0 ) )
        {
            // World MOTD: Open in current world assets only.
            // Ditto world-wide program and scene-wide programs.
            success = tryOpen( openMode, linkedItem, m_worldMetadata.m_worldID, &m_worldMetadata.m_itemKey[0] );
        }
        else if( m_name == _servermessage )
        {
            // Server MOTD: Open in shared assets only.
            if( !m_sharedAssetsWorldID.empty() &&
                m_sharedAssetsItemKey )
            {
                success = tryOpen( openMode, linkedItem, m_sharedAssetsWorldID, m_sharedAssetsItemKey );
            }
        }
        else
        {
            // Normal case: Open in current world assets, with fallback to
            // shared assets if not found.
            success = tryOpen( openMode, linkedItem, m_worldMetadata.m_worldID, &m_worldMetadata.m_itemKey[0] );

            if( !success &&
                !m_sharedAssetsWorldID.empty() &&
                m_sharedAssetsItemKey )
            {
                success = tryOpen( openMode, linkedItem, m_sharedAssetsWorldID, m_sharedAssetsItemKey );
            }
        }
    }
    else
    {
        // Don't permit writing to shared assets.
        success = tryOpen( openMode, linkedItem, m_worldMetadata.m_worldID, &m_worldMetadata.m_itemKey[0] );
    }

    return success;
}

int Encrypted::read( char* data, int offset, int len )
{
    return( m_blockEncryptor->read( data, offset, len ) );
}

int Encrypted::write( const char* data, int offset, int len )
{
    return( m_blockEncryptor->write( data, offset, len ) );
}

bool Encrypted::close()
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "EncryptedAssetLoader: Closing" );
#endif
    m_blockEncryptor->close();
    return( !m_backingLoader || m_backingLoader->close() );
}

int Encrypted::size()
{
    if( m_backingLoader )
    {
        return( m_backingLoader->size() - m_blockEncryptor->overhead() );
    }

    return 0;
}

bool Encrypted::move( const String& newName )
{
    if( !m_backingLoader )
    {
        String loadName( m_name );
        if( m_encryptName && m_hash )
        {
            loadName = encryptedAssetName( &m_worldMetadata.m_itemKey[0], m_name );
        }

        m_backingLoader = m_backingLoaderFactory.makeLoader( Coordinates( m_worldMetadata.m_worldID ), loadName );
    }

    String sendNewName( newName );
    if( m_encryptName && m_hash )
    {
        // Assume non-shared assets.
        sendNewName = encryptedAssetName( &m_worldMetadata.m_itemKey[0], newName );
    }
    return( m_backingLoader->move( sendNewName ) );
}

bool Encrypted::erase()
{
    if( !m_backingLoader )
    {
        String loadName( m_name );
        if( m_encryptName && m_hash )
        {
            loadName = encryptedAssetName( &m_worldMetadata.m_itemKey[0], m_name );
        }

        m_backingLoader = m_backingLoaderFactory.makeLoader( Coordinates( m_worldMetadata.m_worldID ), loadName );
    }

    return( m_backingLoader->erase() );
}

bool Encrypted::error()
{
    return( m_backingLoader->error() );
}

String Encrypted::encryptedAssetName( const char* itemKey,
                                      const String& assetNamePlain )
{
    return Encryptors::Utils::SecureIdentifier::createIdentifier( *m_encryptor,
                                                                  *m_hash,
                                                                  itemKey,
                                                                  assetNamePlain );
}

bool Encrypted::tryOpen( enum OpenMode openMode,
                         const String& linkedItem,
                         const String& worldID,
                         const char* itemKey )
{
    bool success( false );

    enum BlockEncryptor::OpenMode blockEncryptorOpenMode(
        ( openMode == modeRead ) ? BlockEncryptor::modeRead : BlockEncryptor::modeWrite
    );

    String loadName( m_name );
    if( m_encryptName && m_hash )
    {
        loadName = encryptedAssetName( itemKey, m_name );
    }

    m_backingLoader = m_backingLoaderFactory.makeLoader( Coordinates( worldID ), loadName );
    success = m_backingLoader->open( openMode, linkedItem );
    if( success )
    {
        m_blockEncryptor->setKey( itemKey );
        success = m_blockEncryptor->open( blockEncryptorOpenMode, m_backingLoader );
    }

    if( !success )
    {
        m_backingLoader->close();
        delete( m_backingLoader );
        m_backingLoader = nullptr;
    }

    return success;
}

} // namespace AssetLoaders

} // namespace Agape
