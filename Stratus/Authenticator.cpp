#include "Databases/MongoDB/MongoDB.h"
#include "Databases/MongoDB/MongoDocumentBuilder.h"
#include "Loggers/Logger.h"
#include "Utils/base64/base64.h"
#include "World/User.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "Authenticator.h"
#include "Collections.h"
#include "KeyUtilities.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"
#include "Value.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/exception/exception.hpp>

using namespace Agape::Linda2;

using namespace Agape::Databases::MongoDB;

using namespace bsoncxx::builder::stream;
using namespace mongocxx;

namespace Agape
{

namespace Stratus
{

Authenticator::Authenticator( TupleRouter& tupleRouter, KeyUtilities& keyUtilities ) :
  m_tupleRouter( tupleRouter ),
  m_keyUtilities( keyUtilities ),
  m_credentialsValid( false ),
  m_isTela( false ),
  Native( "Authenticator" )
{
    m_tupleRouter.registerActor( this );
}

Authenticator::~Authenticator()
{
    m_tupleRouter.deregisterActor( this );
}

bool Authenticator::accept( Tuple& tuple )
{
    bool handled( false );

    String tupleType( TupleRouter::tupleType( tuple ) );
    if( tupleType == _Authenticate )
    {
        if( tuple.hasValue( _accountAuthKey ) && tuple.hasValue( _deviceAuthKey ) )
        {
            bool success( true );

            String accountAuthKeyEncoded = tuple[_accountAuthKey];
            char accountAuthKeyBin[Base64decode_len( accountAuthKeyEncoded.c_str() )];
            int decodedLen( Base64decode( accountAuthKeyBin, accountAuthKeyEncoded.c_str() ) );
            if( decodedLen == m_keyUtilities.accountSubKeySize() )
            {
                // Hash and re-encode to Base64.
                String accountAuthKeyHashBin( m_keyUtilities.keyHashSize(), '\0' );
                m_keyUtilities.getAccountAuthKeyHash( accountAuthKeyBin, &accountAuthKeyHashBin[0] );

                m_accountAuthKeyHash.resize( Base64encode_len( m_keyUtilities.keyHashSize() ), '\0' );
                Base64encode( &m_accountAuthKeyHash[0], &accountAuthKeyHashBin[0], m_keyUtilities.keyHashSize() );
                m_accountAuthKeyHash.resize( m_accountAuthKeyHash.length() - 1 );

                LOG_DEBUG( "Authenticator: Received account auth key" );
            }
            else
            {
                LOG_DEBUG( "Authenticator: Base64-decoded account auth key not expected length!" );
                success = false;
            }

            String deviceAuthKeyEncoded = tuple[_deviceAuthKey];
            char deviceAuthKeyBin[Base64decode_len( deviceAuthKeyEncoded.c_str() )];
            decodedLen = Base64decode( deviceAuthKeyBin, deviceAuthKeyEncoded.c_str() );
            if( decodedLen == m_keyUtilities.deviceAuthKeySize() )
            {
                // Hash and re-encode to Base64.
                String deviceAuthKeyHashBin( m_keyUtilities.keyHashSize(), '\0' );
                m_keyUtilities.getDeviceAuthKeyHash( deviceAuthKeyBin, &deviceAuthKeyHashBin[0] );

                m_deviceAuthKeyHash.resize( Base64encode_len( m_keyUtilities.keyHashSize() ), '\0' );
                Base64encode( &m_deviceAuthKeyHash[0], &deviceAuthKeyHashBin[0], m_keyUtilities.keyHashSize() );
                m_deviceAuthKeyHash.resize( m_deviceAuthKeyHash.length() - 1 );

                LOG_DEBUG( "Authenticator: Received device auth key" );
            }
            else
            {
                LOG_DEBUG( "Authenticator: Base64-decoded device auth key not expected length!" );
                success = false;
            }

            if( success )
            {
                validateCredentials();
                cacheUserSnowflakes();
            }
        }
        else
        {
            LOG_DEBUG( "Authenticator: Authenticate tuple missing account auth key and/or device auth key!" );
        }

        handled = true;
    }
    else if( ( tupleType == _WorldCreateResponse ) ||
             ( tupleType == _WorldJoinResponse ) )
    {
        cacheUserSnowflakes();
    }

    return handled;
}

bool Authenticator::credentialsValid() const
{
    return m_credentialsValid;
}

bool Authenticator::isTela() const
{
    return m_isTela;
}

const String& Authenticator::accountAuthKeyHash() const
{
    return m_accountAuthKeyHash;
}

const String& Authenticator::deviceAuthKeyHash() const
{
    return m_deviceAuthKeyHash;
}

bool Authenticator::joinedWorld( const String& worldID )
{
    for( auto joinedWorldID : m_joinedWorldIDs )
    {
        if( joinedWorldID == worldID ) return true;
    }

    bool joined( false );
    bool writable( false );
    hasJoinedWorld( worldID, joined, writable );
    if( joined )
    {
        m_joinedWorldIDs.push_back( worldID );
    }
    if( writable )
    {
        m_writableWorldIDs.push_back( worldID );
    }

    return joined;
}
 
bool Authenticator::writableWorld( const String& worldID )
{
    for( auto writableWorldID : m_writableWorldIDs )
    {
        if( writableWorldID == worldID ) return true;
    }

    bool joined( false );
    bool writable( false );
    hasJoinedWorld( worldID, joined, writable );
    if( joined )
    {
        m_joinedWorldIDs.push_back( worldID );
    }
    if( writable )
    {
        m_writableWorldIDs.push_back( worldID );
    }

    return writable;
}

bool Authenticator::isOurUser( const String& snowflake ) const
{
    Vector< String >::const_iterator it( m_userSnowflakes.begin() );
    for( ; it != m_userSnowflakes.end(); ++it )
    {
        if( *it == snowflake )
        {
            return true;
        }
    }

    return false;
}

void Authenticator::validateCredentials()
{
    bool validated( false );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Accounts] );

        LOG_DEBUG( "Authenticator: Validating credentials: Account ID: " + m_accountAuthKeyHash + " Device ID: " + m_deviceAuthKeyHash );

        // Try to find an account with the currently claimed
        // accountAuthKeyHash and deviceAuthKeyHash.
        bsoncxx::stdx::optional< bsoncxx::document::value > account(
            collection.find_one(
                document() << "authKeyHash" << m_accountAuthKeyHash
                           << "devices.authKeyHash" << m_deviceAuthKeyHash << finalize
            )
        );

        if( account )
        {
            LOG_DEBUG( "Authenticator: Authenticated successfully" );
            validated = true;

            Value accountValue( DocumentBuilder::unbuild( *account ) );
            m_isTela = ( accountValue[_telaDeviceAuthKeyHash] == m_deviceAuthKeyHash );
        }
        else
        {
            LOG_DEBUG( "Authenticator: Account with ID " + m_accountAuthKeyHash + " and device with ID " + m_deviceAuthKeyHash + " not found." );
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "Authenticator: Exception attempting to find account with ID " + m_accountAuthKeyHash + " and device with ID " + m_deviceAuthKeyHash + ": " + String( e.what() ) );
    }

    m_credentialsValid = validated;
}

void Authenticator::cacheUserSnowflakes()
{
    LOG_DEBUG( "Authenticator: Caching user snowflakes" );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Accounts] );

        mongocxx::pipeline pipeline;
        pipeline.match( document() << "authKeyHash" << accountAuthKeyHash()
                                   << finalize );
        pipeline.unwind( "$devices" );
        if( !isTela() )
        {
            pipeline.match( document() << "devices.authKeyHash" << deviceAuthKeyHash()
                                       << finalize );
        }
        pipeline.unwind( "$devices.joinedWorlds" );
        pipeline.group( document() << "_id" << int(0)
                                   << "snowflake" << open_document
                                       << "$addToSet" << "$devices.joinedWorlds.user.snowflake"
                                   << close_document
                                   << finalize );

        mongocxx::cursor aggregateCursor(
            collection.aggregate( pipeline )
        );

        bsoncxx::document::view resultView( *aggregateCursor.begin() );
        Value resultValue( DocumentBuilder::unbuild( resultView ) );
        Value& snowflakes( resultValue[_snowflake] );
        m_userSnowflakes.clear();
        ConstListIterator it( snowflakes.listBegin() );
        for( ; it != snowflakes.listEnd(); ++it )
        {
            m_userSnowflakes.push_back( **it );
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "Authenticator: Exception caching user snowflakes: " + String( e.what() ) );
    }
}

void Authenticator::hasJoinedWorld( const String& worldID, bool& joined, bool& writable )
{
    // FIXME: There is a lot of commonality with MongoWorldLoader - we
    // perhaps need some sort of generic accounts collection code that
    // both can share.

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Accounts] );

        LOG_DEBUG( "Authenticator: Looking for joined world " + worldID );

        // Look for account for current accountAuthKeyHash.
        bsoncxx::stdx::optional< bsoncxx::document::value > account(
            collection.find_one(
                document() << "authKeyHash" << m_accountAuthKeyHash << finalize
            )
        );

        if( account )
        {
            Value accountValue( DocumentBuilder::unbuild( *account ) );
            
            // Look for device for current deviceAuthKeyHash.
            Value& devicesValue( accountValue[_devices] );
            Vector< Value* >::const_iterator devicesIt( devicesValue.listBegin() );
            for( ; devicesIt != devicesValue.listEnd(); ++devicesIt )
            {
                // Try to find this worldID in the list of joined worlds for this device.
                hasJoinedWorldWithDevice( *devicesIt, worldID, joined, writable );
                if( joined ) break;
            }
        }
        else
        {
            LOG_DEBUG( "Authenticator: Account with ID " + m_accountAuthKeyHash + " not found." );
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "Authenticator: Exception loading account: " + String( e.what() ) );
    }
}

void Authenticator::hasJoinedWorldWithDevice( const Value* deviceValue, const String& worldID, bool& joined, bool& writable )
{
    auto client( MongoDB::pool().acquire() );
    auto worldsCollection( ( *client )[_Agape][_Worlds] );
    
    // Iterate joined worlds list for device.
    const Value& joinedWorldsValue( ( *deviceValue )[_joinedWorlds] );
    Vector< Value* >::const_iterator joinedWorldsIt( joinedWorldsValue.listBegin() );
    for( ; joinedWorldsIt != joinedWorldsValue.listEnd(); ++joinedWorldsIt )
    {
        String joinedWorldID = ( **joinedWorldsIt )[_worldID];
        String accountUserSnowflake = ( **joinedWorldsIt )[_user][_snowflake];

        if( joinedWorldID == worldID )
        {
            // WorldID in joined worlds for account.
            LOG_DEBUG( "Authenticator: Found joined." );
            joined = true;

            try
            {
                // Load world and check if user is in the world's user list -
                // if so, this world is writable for this account.
                bsoncxx::stdx::optional< bsoncxx::document::value > world(
                    worldsCollection.find_one(
                        document() << "worldID" << joinedWorldID << finalize
                    )
                );

                if( world )
                {
                    Value metadataValue( DocumentBuilder::unbuild( *world ) );
                    World::Metadata metadata( World::Metadata::fromValue( metadataValue ) );
                    for( auto worldUser : metadata.m_users )
                    {
                        if( worldUser.m_snowflake == accountUserSnowflake )
                        {
                            writable = true;
                            LOG_DEBUG( "Authenticator: World is writable." );
                            break;
                        }
                    }
                }
                else
                {
                    LOG_DEBUG( "Authenticator: Unable to find world for joined world ID " + joinedWorldID );
                }
            }
            catch( mongocxx::exception& e )
            {
                LOG_DEBUG( "Authenticator: Exception loading world with ID " + joinedWorldID + ": " + String( e.what() ) );
            }

            break;
        }
    }
}

} // namespace Stratus

} // namespace Agape
