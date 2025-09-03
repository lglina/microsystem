#include "Databases/MongoDB/MongoDB.h"
#include "Databases/MongoDB/MongoDocumentBuilder.h"
#include "Encryptors/Utils/SecureIdentifier.h"
#include "Loggers/Logger.h"
#include "Utils/base64/base64.h"
#include "World/WorldCoordinates.h"
#include "Authenticator.h"
#include "MongoAssetLoader.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/exception/exception.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>


using namespace Agape::Databases::MongoDB;

using namespace bsoncxx::builder::stream;
using namespace mongocxx;

namespace Agape
{

using namespace Stratus;

namespace AssetLoaders
{

Mongo::Mongo( const World::Coordinates& coordinates,
              const String& name,
              const String& collectionName,
              bool encryptedNames,
              Authenticator& authenticator ) :
  m_isOpen( false ),
  m_openMode( modeRead ),
  m_collectionName( collectionName ),
  m_encryptedNames( encryptedNames ),
  m_authenticator( authenticator ),
  AssetLoader( coordinates, name )
{
}

bool Mongo::open()
{
    return open( modeRead, String() );
}

bool Mongo::open( enum OpenMode openMode, const String& linkedItem )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoAssetLoader: Attempting to open asset " + m_name );
#endif

    m_linkedItem = linkedItem; // Set on close() if opened for write.

    bool success( true );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][m_collectionName.c_str()] );

        // Look for existing.
        bsoncxx::stdx::optional< bsoncxx::document::value > assetDocument;
        if( m_encryptedNames )
        {
            String hashedAssetName;
            String encryptedAssetName;
            Encryptors::Utils::SecureIdentifier::splitIdentifier( m_name, hashedAssetName, encryptedAssetName );
#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoAssetLoader: Open: Hashed name: " + hashedAssetName );
            LOG_DEBUG( "MongoAssetLoader: Open: Encrypted name: " + encryptedAssetName );
#endif
            assetDocument =
                collection.find_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID 
                               << "hash" << hashedAssetName
                               << finalize
                );
        }
        else
        {
            assetDocument =
                collection.find_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID 
                               << "name" << m_name
                               << finalize
                );
        }

        if( openMode == modeRead )
        {
            if( assetDocument )
            {
#ifdef LOG_LOADERS
                LOG_DEBUG( "MongoAssetLoader: Opened for reading." );
#endif
                //LOG_DEBUG( bsoncxx::to_json( assetDocument->view() ).c_str() );
                Value assetValue( DocumentBuilder::unbuild( *assetDocument ) );

                m_readBuffer = assetValue[_data];
            }
            else
            {
                LOG_DEBUG( "MongoAssetLoader: Error: Asset not found while opening for reading." );
                success = false;
            }
        }
        else if( openMode == modeWrite )
        {
            if( m_authenticator.writableWorld( m_coordinates.m_worldID ) )
            {
                if( !assetDocument )
                {
#ifdef LOG_LOADERS
                    LOG_DEBUG( "MongoAssetLoader: Opened for writing." );
#endif
                }
                else
                {
                    LOG_DEBUG( "MongoAssetLoader: Error: Asset already exists while opening for writing." );
                    success = false;
                }
            }
            else
            {
                LOG_DEBUG( "MongoAssetLoader: Error: Attemped to open for write but world not writable for this user." );
                success = false;
            }
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoAssetLoader: Exception opening asset: " + String( e.what() ) );
        success = false;
    }

    m_openMode = openMode;
    m_isOpen = success;

    return success;
}

int Mongo::read( char* data, int offset, int len )
{
    if( m_isOpen &&
      ( m_openMode == modeRead ) &&
      ( offset >= 0 ) &&
      ( offset < m_readBuffer.length() ) )
    {
        int lenToRead = ( ( offset + len ) > m_readBuffer.length() ) ? ( m_readBuffer.length() - offset ) : len;
        ::memcpy( data, &m_readBuffer[0] + offset, lenToRead );
        return lenToRead;
    }

    return 0;
}

int Mongo::write( const char* data, int offset, int len )
{
    if( m_writeBuffer.length() < ( offset + len ) )
    {
        m_writeBuffer.resize( offset + len );
    }
    ::memcpy( &m_writeBuffer[0] + offset, data, len );
    return len;
}

bool Mongo::close()
{
    bool success( true );

    if( m_isOpen && ( m_openMode == modeWrite ) )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "MongoAssetLoader: Saving buffered asset data" );
#endif

        try
        {
            auto client( MongoDB::pool().acquire() );
            auto collection( ( *client )[_Agape][m_collectionName.c_str()] );

            Value assetValue;
            if( m_encryptedNames )
            {
                String hashedAssetName;
                String encryptedAssetName;
                Encryptors::Utils::SecureIdentifier::splitIdentifier( m_name, hashedAssetName, encryptedAssetName );
#ifdef LOG_LOADERS
                LOG_DEBUG( "MongoAssetLoader: Close: Hashed name: " + hashedAssetName );
                LOG_DEBUG( "MongoAssetLoader: Close: Encrypted name: " + encryptedAssetName );
#endif
                assetValue[_hash] = hashedAssetName;
                assetValue[_name] = encryptedAssetName;
            }
            else
            {
                assetValue[_name] = m_name;
            }
            assetValue[_data] = m_writeBuffer;
            assetValue[_data].markBinary();
            assetValue[_author] = m_authenticator.accountAuthKeyHash();
            assetValue[_linkedItem] = m_linkedItem;
            m_coordinates.toValue( assetValue[_coordinates] );
            collection.insert_one( DocumentBuilder::build( assetValue ) );
        }
        catch( mongocxx::exception& e )
        {
            LOG_DEBUG( "MongoAssetLoader: Exception saving buffered asset data: " + String( e.what() ) );
            success = false;
        }
    }

    return success;
}

int Mongo::size()
{
    return m_readBuffer.length();
}

bool Mongo::move( const String& newName )
{
    if( !m_authenticator.writableWorld( m_coordinates.m_worldID ) )
    {
        LOG_DEBUG( "MongoAssetLoader: Attempted to move but world not writable for this user" );
        return false;
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoAssetLoader: Renaming asset" );
#endif

    bool success( true );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][m_collectionName.c_str()] );

        // Delete existing.
        // FIXME: This is not atomic. Do we care? Do we need a transaction?
        if( m_encryptedNames )
        {
            String hashedAssetName;
            String encryptedAssetName;
            Encryptors::Utils::SecureIdentifier::splitIdentifier( newName, hashedAssetName, encryptedAssetName );
#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoAssetLoader: Move: Delete: Hashed name: " + hashedAssetName );
            LOG_DEBUG( "MongoAssetLoader: Move: Delete: Encrypted name: " + encryptedAssetName );
#endif
            {
            bsoncxx::stdx::optional< mongocxx::result::delete_result > result(
                collection.delete_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID 
                               << "hash" << hashedAssetName << finalize
                )
            );
            }
        }
        else
        {
            {
            bsoncxx::stdx::optional< mongocxx::result::delete_result > result(
                collection.delete_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID 
                               << "name" << newName << finalize
                )
            );
            }
        }

        {
        bsoncxx::stdx::optional< mongocxx::result::update > result;

        Value assetNewValues;
        if( m_encryptedNames )
        {
            String newHashedAssetName;
            String newEncryptedAssetName;
            Encryptors::Utils::SecureIdentifier::splitIdentifier( newName, newHashedAssetName, newEncryptedAssetName );

            String oldHashedAssetName;
            String oldEncryptedAssetName;
            Encryptors::Utils::SecureIdentifier::splitIdentifier( m_name, oldHashedAssetName, oldEncryptedAssetName );

            assetNewValues[_hash] = newHashedAssetName;
            assetNewValues[_name] = newEncryptedAssetName;
            bsoncxx::document::value bsonAssetNewValues( DocumentBuilder::build( assetNewValues ) );

#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoAssetLoader: Move: Old hashed name: " + oldHashedAssetName );
            LOG_DEBUG( "MongoAssetLoader: Move: New hashed name: " + newHashedAssetName );
            LOG_DEBUG( "MongoAssetLoader: Move: New encrypted name: " + newEncryptedAssetName );
#endif

            result =
                collection.update_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID 
                               << "hash" << oldHashedAssetName << finalize,
                    document() << "$set" << bsonAssetNewValues << finalize
                );
        }
        else
        {
            assetNewValues[_name] = newName;
            bsoncxx::document::value bsonAssetNewValues( DocumentBuilder::build( assetNewValues ) );

            result =
                collection.update_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID 
                               << "name" << m_name << finalize,
                    document() << "$set" << bsonAssetNewValues << finalize
                );
            // FIXME: Should we ever expect existing document hash field to have a value?
        }

        if( !result || ( result->modified_count() != 1 ) )
        {
            LOG_DEBUG( "MongoAssetLoader: Asset not renamed" );
            success = false;
        }
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoAssetLoader: Exception renaming asset: " + String( e.what() ) );
        success = false;
    }

    return success;
}

bool Mongo::erase()
{
    if( !m_authenticator.writableWorld( m_coordinates.m_worldID ) )
    {
        LOG_DEBUG( "MongoAssetLoader: Attempted to erase but world not writable for this user" );
        return false;
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoAssetLoader: Deleting asset" );
#endif

    bool success( true );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][m_collectionName.c_str()] );

        bsoncxx::stdx::optional< mongocxx::result::delete_result > result;
        if( m_encryptedNames )
        {
            String hashedAssetName;
            String encryptedAssetName;
            Encryptors::Utils::SecureIdentifier::splitIdentifier( m_name, hashedAssetName, encryptedAssetName );
#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoAssetLoader: Erase: Hashed name: " + hashedAssetName );
            LOG_DEBUG( "MongoAssetLoader: Erase: Encrypted name: " + encryptedAssetName );
#endif
            result =
                collection.delete_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID 
                               << "hash" << hashedAssetName << finalize
                );
        }
        else
        {
            result =
                collection.delete_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID 
                               << "name" << m_name << finalize
                );
        }

        if( !result || ( result->deleted_count() != 1 ) )
        {
            LOG_DEBUG( "MongoAssetLoader: Asset not deleted" );
            success = false;
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoAssetLoader: Exception deleting asset: " + String( e.what() ) );
        success = false;
    }

    return success;
}

} // namespace AssetLoaders

} // namespace Agape
