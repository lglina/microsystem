#include "Databases/MongoDB/MongoDB.h"
#include "Databases/MongoDB/MongoDocumentBuilder.h"
#include "Loggers/Logger.h"
#include "World/Teleport.h"
#include "World/UniverseStats.h"
#include "World/WorldMetadata.h"
#include "World/WorldSummary.h"
#include "Authenticator.h"
#include "MongoWorldLoader.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"
#include "WorldLoader.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/pipeline.hpp>
#include <mongocxx/exception/exception.hpp>

using namespace Agape::Databases::MongoDB;

using namespace bsoncxx::builder::stream;
using namespace mongocxx;

using namespace std::placeholders;

namespace Agape
{

namespace WorldLoaders
{

Mongo::Mongo( Stratus::Authenticator& authenticator ) :
  m_authenticator( authenticator )
{
}

bool Mongo::create( const World::Metadata& metadata, String& reason )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoWorldLoader: Creating world." );
#endif

    bool success( true );

    String privateKey( metadata.m_privateKey );
    World::User user;
    if( metadata.m_users.size() == 1 ) user = metadata.m_users[0]; // Should always be 1!

    Value metadataValue;
    metadata.toValue( metadataValue );
    metadataValue.erase( _privateKey ); // Don't save private key to Worlds collection!
    bsoncxx::document::value bsonDocument( DocumentBuilder::build( metadataValue ) );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Worlds] );

        // Look for existing.
        bsoncxx::stdx::optional< bsoncxx::document::value > result(
            collection.find_one(
                document() << "worldID" << metadata.m_worldID << finalize
            )
        );

        if( !result )
        {
            collection.insert_one( DocumentBuilder::build( metadataValue ) );
#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoWorldLoader: Created." );
#endif
        }
        else
        {
            LOG_DEBUG( "MongoWorldLoader: World already exists." );
            reason = "World already exists";
            success = false;
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoWorldLoader: Exception creating world: " + String( e.what() ) );
        reason = "Server error";
        success = false;
    }

    if( success )
    {
        success = addJoinedWorld( metadata.m_worldID, privateKey, user, reason );
    }

    return success;
}

bool Mongo::join( World::Metadata& metadata, String& reason )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoWorldLoader: Joining world." );
#endif

    bool success( true );

    String privateKey( metadata.m_privateKey );
    World::User user;
    if( metadata.m_users.size() == 1 ) user = metadata.m_users[0]; // Should always be 1!

    if( metadata.m_users.size() == 1 )
    {
        World::Metadata currentMetadata( metadata );
        if( load( currentMetadata, reason ) )
        {
            // Write user to World metadata - allows user to modify world
            // assets, have presence, receive telegrams, etc. Otherwise world is
            // added to user's device and account only, and user is only
            // allowed to visit (invisibly) and not change anything. 
            if( currentMetadata.m_writable )
            {
                Value userValue;
                metadata.m_users[0].toValue( userValue );
                bsoncxx::document::value bsonUser( DocumentBuilder::build( userValue ) );

                try
                {
                    auto client( MongoDB::pool().acquire() );
                    auto collection( ( *client )[_Agape][_Worlds] );

                    options::find_one_and_update options;
                    bsoncxx::stdx::optional< bsoncxx::document::value > world(
                        collection.find_one_and_update(
                            document() << "worldAuthKey" << metadata.m_worldAuthKey << finalize,
                            document() << "$addToSet" << open_document << "users" << bsonUser << close_document << finalize,
                            options.return_document( options::return_document::k_after )
                        )
                    );

                    if( world )
                    {
#ifdef LOG_LOADERS
                        LOG_DEBUG( "MongoWorldLoader: Joined." );
#endif
                        Value metadataValue( DocumentBuilder::unbuild( *world ) );
                        metadata = World::Metadata::fromValue( metadataValue );
                    }
                    else
                    {
                        LOG_DEBUG( "MongoWorldLoader: World not found." );
                        reason = "World not found";
                        success = false;
                    }
                }
                catch( mongocxx::exception& e )
                {
                    LOG_DEBUG( "MongoWorldLoader: Exception joining world: " + String( e.what() ) );
                    reason = "Server error";
                    success = false;
                }
            }
            else
            {
                // Not writable - don't add user to world user list, but
                // set metadata to current metadata and add to user's
                // joined worlds below.
                metadata = currentMetadata;
            }
        }
        else
        {
            LOG_DEBUG( "MongoWorldLoader: Error loading world metadata to check writability: " + reason );
            reason = "Server error";
            success = false;
        }
    }
    else
    {
        LOG_DEBUG( "MongoWorldLoader: Invalid user specification." );
        reason = "Invalid user specification";
        success = false;
    }

    if( success )
    {
        success = addJoinedWorld( metadata.m_worldID, privateKey, user, reason );
    }

    return success;
}

bool Mongo::load( World::Metadata& metadata, String& reason )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoWorldLoader: Loading world." );
#endif

    bool success( true );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Worlds] );

        // Look for existing.
        bsoncxx::stdx::optional< bsoncxx::document::value > world(
            collection.find_one(
                document() << "worldAuthKey" << metadata.m_worldAuthKey << finalize
            )
        );

        if( world )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoWorldLoader: Loaded." );
#endif
            Value metadataValue( DocumentBuilder::unbuild( *world ) );
            metadata = World::Metadata::fromValue( metadataValue );
        }
        else
        {
            LOG_DEBUG( "MongoWorldLoader: World not found." );
            reason = "World not found";
            success = false;
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoWorldLoader: Exception loading world: " + String( e.what() ) );
        success = false;
    }

    return success;
}

bool Mongo::loadJoinedWorlds( Vector< World::Metadata >& joinedWorlds, bool allDevices, String& reason )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoWorldLoader: Loading joined worlds." );
#endif
    const String& accountAuthKeyHash( m_authenticator.accountAuthKeyHash() );
    const String& deviceAuthKeyHash( m_authenticator.deviceAuthKeyHash() );
    return( withDevice( accountAuthKeyHash,
                        deviceAuthKeyHash,
                        allDevices,
                        reason,
                        std::bind( &Mongo::getDeviceJoinedWorlds, this, std::ref( joinedWorlds ), _1, _2 ) ) );
}

bool Mongo::loadTeleports( Vector< World::Teleport >& teleports, bool allDevices, String& reason )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoWorldLoader: Loading teleports." );
#endif
    const String& accountAuthKeyHash( m_authenticator.accountAuthKeyHash() );
    const String& deviceAuthKeyHash( m_authenticator.deviceAuthKeyHash() );
    return( withDevice( accountAuthKeyHash,
                        deviceAuthKeyHash,
                        allDevices,
                        reason,
                        std::bind( &Mongo::getDeviceTeleports, this, std::ref( teleports ), _1, _2 ) ) );
}

bool Mongo::createTeleport( World::Teleport& teleport, String& reason )
{
    // Add this teleport to the current user's teleports (for the
    // current device).

    bool success( false );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Accounts] );

        // We can assume here that the user and device IDs from
        // authenticator are valid, so we just have to create a new
        // entry in teleports for the device.

        const String& accountAuthKeyHash( m_authenticator.accountAuthKeyHash() );
        const String& deviceAuthKeyHash( m_authenticator.deviceAuthKeyHash() );

        Value teleportValue;
        teleport.toValue( teleportValue );

        bsoncxx::document::value bsonTeleport( DocumentBuilder::build( teleportValue ) );

#ifdef LOG_LOADERS
        LOG_DEBUG( "MongoWorldLoader: Saving teleport for account hash " + accountAuthKeyHash + " device hash " + deviceAuthKeyHash );
        LOG_DEBUG( teleportValue.dump() );
#endif

        options::update options;
        bsoncxx::stdx::optional< mongocxx::result::update > result(
            collection.update_one(
                document() << "authKeyHash" << accountAuthKeyHash
                        << "devices.authKeyHash" << deviceAuthKeyHash << finalize,
                document() << "$addToSet" << open_document << "devices.$.teleports" << bsonTeleport << close_document << finalize,
                options.upsert( true )
            )
        );

        success = ( result && ( ( result->matched_count() == 1 ) || ( result->upserted_count() == 1 ) ) );
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoWorldLoader: Exception saving teleport for user: " + String( e.what() ) );
        reason = "Server error";
    }

    return success;
}

bool Mongo::deleteTeleport( World::Teleport& teleport, String& reason )
{
    // Delete this teleport from the current user's teleports (for the
    // current device).

    bool success( false );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Accounts] );

        const String& accountAuthKeyHash( m_authenticator.accountAuthKeyHash() );
        const String& deviceAuthKeyHash( m_authenticator.deviceAuthKeyHash() );

#ifdef LOG_LOADERS
        LOG_DEBUG( "MongoWorldLoader: Deleting teleport for account hash " + accountAuthKeyHash + " device hash " + deviceAuthKeyHash );
#endif

        bsoncxx::stdx::optional< mongocxx::result::update > result(
            collection.update_one(
                document() << "authKeyHash" << accountAuthKeyHash
                           << "devices.authKeyHash" << deviceAuthKeyHash << finalize,
                document() << "$pull" << open_document << "devices.$.teleports"
                                    << open_document << "snowflake" << teleport.m_snowflake
                                    << close_document
                                    << close_document
                                    << finalize
            )
        );
        success = ( result && ( result->modified_count() == 1 ) );
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoWorldLoader: Exception deleting teleport for user: " + String( e.what() ) );
        reason = "Server error";
    }

    return success;
}

bool Mongo::loadWorldSummaries( Vector< World::Summary >& worldSummaries, int from, int size, String& reason )
{
    // FIXME: This whole process is probably horrifically slow and we likely
    // need to cache world item and user counts (either in RAM or in the
    // database itself...). Also, given loadJoinedWorlds only returns a single
    // user (the current user) for each world, we need to do a second lookup
    // for each joined world, which will slow us down.
    bool success( true );

    Vector< World::Metadata > joinedWorlds;
    success = loadJoinedWorlds( joinedWorlds, false, reason );

    Vector< World::Summary > allWorldSummaries;

    if( success )
    {
        Vector< World::Metadata >::const_iterator joinedWorldsIt( joinedWorlds.begin() );
        for( ; joinedWorldsIt != joinedWorlds.end(); ++joinedWorldsIt )
        {
            int itemCount( 0 );
            if( !worldItemCount( joinedWorldsIt->m_worldID, itemCount, reason ) )
            {
                success = false;
                break;
            }

            // Load just this world, as loadJoinedWorlds doesn't give us all
            // users, but this will.
            World::Metadata thisWorldMetadata;
            thisWorldMetadata.m_worldAuthKey = joinedWorldsIt->m_worldAuthKey;
            if( load( thisWorldMetadata, reason ) )
            {
                World::Summary summary;
                summary.m_worldID = joinedWorldsIt->m_worldID;
                summary.m_users = thisWorldMetadata.m_users.size();
                summary.m_items = itemCount;
                allWorldSummaries.push_back( summary );
            }
            else
            {
                success = false;
                break;
            }
        }

        if( success )
        {
            try
            {
                auto client( MongoDB::pool().acquire() );
                auto worldsCollection( ( *client )[_Agape][_Worlds] );
                
                mongocxx::cursor distinctCursor(
                    worldsCollection.distinct( "worldID",
                                            document() << finalize )
                );

                bsoncxx::document::view resultView( *distinctCursor.begin() );
                Value resultValue( DocumentBuilder::unbuild( resultView ) );
                Value resultValueValues( resultValue[_values] );
                
                ListIterator listIt( resultValueValues.listBegin() );
                for( ; listIt != resultValueValues.listEnd(); ++listIt )
                {
                    // FIXME: O(n^2)! Refactor.
                    Vector< World::Summary >::const_iterator existingIt( allWorldSummaries.begin() );
                    for( ; existingIt != allWorldSummaries.end(); ++existingIt )
                    {
                        if( existingIt->m_worldID == **listIt )
                        {
                            break;
                        }
                    }

                    if( existingIt == allWorldSummaries.end() )
                    {
                        int userCount( 0 );
                        int itemCount( 0 );

                        if( worldUserCount( **listIt, userCount, reason ) &&
                            worldItemCount( **listIt, itemCount, reason ) )
                        {
                            World::Summary summary;
                            summary.m_worldID = **listIt;
                            summary.m_users = userCount;
                            summary.m_items = itemCount;
                            allWorldSummaries.push_back( summary );
                        }
                        else
                        {
                            success = false;
                            break;
                        }
                    }
                }
            }
            catch( mongocxx::exception& e )
            {
                LOG_DEBUG( "MongoWorldLoader: Exception loading world IDs: " + String( e.what() ) );
                success = false;
            }
        }
    }

    if( success )
    {
        for( int idx = 0; ( ( idx < ( from + size ) ) && ( idx < allWorldSummaries.size() ) ); ++idx )
        {
            if( idx >= from )
            {
                worldSummaries.push_back( allWorldSummaries[idx] );
            }
        }
    }

    return success;
}

bool Mongo::loadUniverseStats( World::UniverseStats& universeStats, String& reason )
{
    return( worldItemCount( String(), universeStats.m_items, reason ) ); // With no world ID, will get universe count.
}

bool Mongo::addJoinedWorld( const String& worldID,
                            const String& privateKey,
                            const World::User& user,
                            String& reason )
{
    // Add the private key and user details for this world to the current
    // user's joinedWorlds (for the current device).

    bool success( false );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Accounts] );

        // We can assume here that the user and device IDs from
        // authenticator are valid, so we just have to create a new
        // entry in joinedWorlds for the device.

        const String& accountAuthKeyHash( m_authenticator.accountAuthKeyHash() );
        const String& deviceAuthKeyHash( m_authenticator.deviceAuthKeyHash() );

        Value joinedWorldValue;
        joinedWorldValue[_worldID] = worldID;
        joinedWorldValue[_privateKey] = privateKey;
        user.toValue( joinedWorldValue[_user] );

        bsoncxx::document::value bsonJoinedWorld( DocumentBuilder::build( joinedWorldValue ) );

#ifdef LOG_LOADERS
        LOG_DEBUG( "MongoWorldLoader: Saving world private key for account hash " + accountAuthKeyHash + " device hash " + deviceAuthKeyHash );
        LOG_DEBUG( joinedWorldValue.dump() );
#endif

        options::update options;
        bsoncxx::stdx::optional< mongocxx::result::update > result(
            collection.update_one(
                document() << "authKeyHash" << accountAuthKeyHash
                        << "devices.authKeyHash" << deviceAuthKeyHash << finalize,
                document() << "$addToSet" << open_document << "devices.$.joinedWorlds" << bsonJoinedWorld << close_document << finalize,
                options.upsert( true )
            )
        );

        success = ( result && ( ( result->matched_count() == 1 ) || ( result->upserted_count() == 1 ) ) );
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoWorldLoader: Exception saving world private key for user: " + String( e.what() ) );
        reason = "Server error";
    }

    return success;
}

bool Mongo::withDevice( const String& accountAuthKeyHash,
                        const String& deviceAuthKeyHash,
                        bool allDevices,
                        String& reason,
                        std::function< bool( const Value*, String& ) > deviceCallback )
{
    bool success( true );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Accounts] );

        // Look for existing.
        bsoncxx::stdx::optional< bsoncxx::document::value > account(
            collection.find_one(
                document() << "authKeyHash" << accountAuthKeyHash << finalize
            )
        );

        if( account )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoWorldLoader: Found account." );
#endif
            Value accountValue( DocumentBuilder::unbuild( *account ) );
            
            Value& devicesValue( accountValue[_devices] );
            Vector< Value* >::const_iterator devicesIt( devicesValue.listBegin() );
            for( ; ( success && ( devicesIt != devicesValue.listEnd() ) ); ++devicesIt )
            {
                if( allDevices || ( ( **devicesIt )[_authKeyHash] == deviceAuthKeyHash ) )
                {
                    success = deviceCallback( *devicesIt, reason );
                    if( !allDevices ) break;
                }
            }

            if( !allDevices && ( devicesIt == devicesValue.listEnd() ) )
            {
                LOG_DEBUG( "MongoWorldLoader: Device with ID " + deviceAuthKeyHash + " not found in account " + accountAuthKeyHash + "." );
                reason = "Device with ID " + deviceAuthKeyHash + " not found in account " + accountAuthKeyHash + ".";
                success = false;
            }
        }
        else
        {
            LOG_DEBUG( "MongoWorldLoader: Account with ID " + accountAuthKeyHash + " not found." );
            reason = "Account with ID " + accountAuthKeyHash + " not found.";
            success = false;
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoWorldLoader: Exception loading account: " + String( e.what() ) );
        success = false;
    }

    return success;
}

bool Mongo::getDeviceJoinedWorlds( Vector< World::Metadata >& joinedWorlds, const Value* deviceValue, String& reason )
{
    bool success( true );

    auto client( MongoDB::pool().acquire() );
    auto worldsCollection( ( *client )[_Agape][_Worlds] );
    
    const Value& joinedWorldsValue( ( *deviceValue )[_joinedWorlds] );
    Vector< Value* >::const_iterator joinedWorldsIt( joinedWorldsValue.listBegin() );
    for( ; ( success && ( joinedWorldsIt != joinedWorldsValue.listEnd() ) ); ++joinedWorldsIt )
    {
        String worldID = ( **joinedWorldsIt )[_worldID];
        
        try
        {
            // Look for world.
            bsoncxx::stdx::optional< bsoncxx::document::value > world(
                worldsCollection.find_one(
                    document() << "worldID" << worldID << finalize
                )
            );

            if( world )
            {
                Value metadataValue( DocumentBuilder::unbuild( *world ) );
                World::Metadata metadata( World::Metadata::fromValue( metadataValue ) );
                metadata.m_users.clear();
                metadata.m_users.push_back( World::User::fromValue( ( **joinedWorldsIt )[_user] ) ); // Replace all users with this user.
                metadata.m_privateKey = ( **joinedWorldsIt )[_privateKey];
                joinedWorlds.push_back( metadata );
            }
            else
            {
                LOG_DEBUG( "MongoWorldLoader: Unable to find world for joined world ID " + worldID );
                reason = "Joined world with ID " + worldID + " not found";
                success = false;
            }
        }
        catch( mongocxx::exception& e )
        {
            LOG_DEBUG( "MongoWorldLoader: Exception loading world with ID " + worldID + ": " + String( e.what() ) );
            success = false;
        }
    }

    return success;
}

bool Mongo::getDeviceTeleports( Vector< World::Teleport >& teleports, const Value* deviceValue, String& reason )
{
    if( deviceValue->hasValue( _teleports ) )
    {
        const Value& teleportsValue( ( *deviceValue )[_teleports] );
        Vector< Value* >::const_iterator teleportsIt( teleportsValue.listBegin() );
        for( ; teleportsIt != teleportsValue.listEnd(); ++teleportsIt )
        {
            World::Teleport teleport( World::Teleport::fromValue( **teleportsIt ) );
            teleports.push_back( teleport );
        }
    }

    return true;
}

bool Mongo::worldUserCount( const String& worldID, int& count, String& reason )
{
    bool success( true );

    // FIXME: Could use an aggregation with $size here?
    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Worlds] );

        bsoncxx::stdx::optional< bsoncxx::document::value > world(
            collection.find_one(
                document() << "worldID" << worldID << finalize
            )
        );

        if( world )
        {
            Value metadataValue( DocumentBuilder::unbuild( *world ) );
            World::Metadata metadata( World::Metadata::fromValue( metadataValue ) );
            count = metadata.m_users.size();
        }
        else
        {
            LOG_DEBUG( "MongoWorldLoader: World not found getting user count for " + worldID );
            reason = "Server error";
            count = 0;
            success = false;
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoWorldLoader: Exception getting user count for world " + worldID + ": " + String( e.what() ) );
        reason = "Server error";
        count = 0;
        success = false;
    }

    return success;
}

bool Mongo::worldItemCount( const String& worldID, int& count, String& reason )
{
    bool success( true );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Scenes] );

        mongocxx::pipeline pipeline;
        if( !worldID.empty() )
        {
            pipeline.match( document() << "coordinates.worldID" << worldID
                                       << finalize );
        } // Else get universe item count.
        pipeline.project( document() << "_id" << int( 0 )
                                     << "sceneItems" << open_document << "$size" << "$sceneItems"
                                                                      << close_document
                                     << finalize );
        pipeline.group( document() << "_id" << ""
                                   << "itemCount" << open_document << "$sum" << "$sceneItems"
                                                                   << close_document
                                   << finalize );

        mongocxx::cursor aggregateCursor(
            collection.aggregate( pipeline )
        );

        bsoncxx::document::view resultView( *aggregateCursor.begin() );
        Value resultValue( DocumentBuilder::unbuild( resultView ) );
        count = resultValue["itemCount"];
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoWorldLoader: Exception getting item count for world " + worldID + ": " + String( e.what() ) );
        reason = "Server error";
        count = 0;
        success = false;
    }

    return success;
}

} // namespace WorldLoaders

} // namespace Agape
