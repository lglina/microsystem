#include "Databases/MongoDB/MongoDB.h"
#include "Databases/MongoDB/MongoDocumentBuilder.h"
#include "Loggers/Logger.h"
#include "World/Telegram.h"
#include "Authenticator.h"
#include "Collections.h"
#include "MongoTelegramLoader.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/pipeline.hpp>
#include <mongocxx/exception/exception.hpp>

using namespace Agape::Databases::MongoDB;
using namespace Agape::Stratus;
using namespace Agape::World;

using namespace bsoncxx::builder::stream;
using namespace mongocxx;

namespace Agape
{

namespace TelegramLoaders
{

Mongo::Mongo( const String& recipientSnowflake,
              Authenticator& authenticator ) :
  m_authenticator( authenticator ),
  TelegramLoader( recipientSnowflake )
{
}

bool Mongo::load( Vector< Telegram >& telegrams )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoTelegramLoader: Loading telegrams for " + m_recipientSnowflake );
#endif

    bool success( true );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Telegrams] );

        // Look for existing.
        mongocxx::cursor telegramCursor(
            collection.find(
                document() << "recipientSnowflake" << m_recipientSnowflake
                           << finalize
            )
        );

        mongocxx::cursor::iterator it( telegramCursor.begin() );
        for( ; it != telegramCursor.end(); ++it )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoTelegramLoader: Loaded telegram." );
            LOG_DEBUG( bsoncxx::to_json( *it ).c_str() );
#endif
            Value telegramValue( DocumentBuilder::unbuild( *it ) );
            Telegram telegram( Telegram::fromValue( telegramValue ) );
            telegrams.push_back( telegram );
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoTelegramLoader: Exception loading telegrams: " + String( e.what() ) );
        success = false;
    }

    return success;
}

bool Mongo::loadSent( Vector< Telegram >& telegrams )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoTelegramLoader: Loading sent telegrams for " + m_recipientSnowflake );
#endif

    bool success( true );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Telegrams] );

        // Look for existing.
        mongocxx::cursor telegramCursor(
            collection.find(
                document() << "senderSnowflake" << m_recipientSnowflake
                           << finalize
            )
        );

        mongocxx::cursor::iterator it( telegramCursor.begin() );
        for( ; it != telegramCursor.end(); ++it )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoTelegramLoader: Loaded sent telegram." );
            LOG_DEBUG( bsoncxx::to_json( *it ).c_str() );
#endif
            Value telegramValue( DocumentBuilder::unbuild( *it ) );
            Telegram telegram( Telegram::fromValue( telegramValue ) );
            telegrams.push_back( telegram );
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoTelegramLoader: Exception loading sent telegrams: " + String( e.what() ) );
        success = false;
    }

    return success;
}

bool Mongo::send( const Telegram& telegram )
{
    // FIXME: Should check for the rare case of duplicate telegram snowflake
    // on insert. Speaking of which, we should do this in all cases where
    // snowflake collision might occur (due to re-use of machine numbers).
#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoTelegramLoader: Saving (sending) telegram" );
#endif

    bool success( true );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Telegrams] );

        Value telegramValue;
        telegram.toValue( telegramValue );
        collection.insert_one( DocumentBuilder::build( telegramValue ) );
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoTelegramLoader: Exception saving telegram: " + String( e.what() ) );
        success = false;
    }

    return success;
}

bool Mongo::markRead( const Telegram& telegram )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoTelegramLoader: Marking telegram read" );
#endif

    bool success( true );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Telegrams] );

        Value telegramNewValues;
        telegramNewValues[_unread] = 0;
        bsoncxx::document::value bsonTelegramNewValues( DocumentBuilder::build( telegramNewValues ) );

        bsoncxx::stdx::optional< mongocxx::result::update > result(
            collection.update_one(
                document() << "telegramSnowflake" << telegram.m_telegramSnowflake
                           << "recipientSnowflake" << m_recipientSnowflake // Only allowed to mark our own!,
                           << finalize,
                document() << "$set" << bsonTelegramNewValues << finalize
            )
        );

        if( !result || ( result->matched_count() != 1 ) )
        {
            LOG_DEBUG( "MongoTelegramLoader: Telegram not marked read" );
            success = false;
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoTelegramLoader: Exception marking telegram read: " + String( e.what() ) );
        success = false;
    }

    return success;
}

bool Mongo::erase( const Telegram& telegram )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoTelegramLoader: Deleting telegram" );
#endif

    bool success( true );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Telegrams] );

        bsoncxx::stdx::optional< mongocxx::result::delete_result > result(
            collection.delete_one(
                document() << "telegramSnowflake" << telegram.m_telegramSnowflake
                           << "recipientSnowflake" << m_recipientSnowflake // Only allowed to delete our own!
                           << finalize
            )
        );

        if( !result || ( result->deleted_count() != 1 ) )
        {
            LOG_DEBUG( "MongoTelegramLoader: Telegram not deleted" );
            success = false;
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoTelegramLoader: Exception deleting telegram: " + String( e.what() ) );
        success = false;
    }

    return success;
}

bool Mongo::unread( Map< String, int >& numUnread, bool allDevices )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoTelegramLoader: Looking for unread telegrams" );
#endif

    bool success( true );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Accounts] );

        mongocxx::pipeline pipeline;
        pipeline.match( document() << "authKeyHash" << m_authenticator.accountAuthKeyHash()
                                   << finalize );
        pipeline.unwind( "$devices" );
        if( !allDevices )
        {
            pipeline.match( document() << "devices.authKeyHash" << m_authenticator.deviceAuthKeyHash()
                                       << finalize );
        }
        pipeline.unwind( "$devices.joinedWorlds" );
        pipeline.lookup( document() << "from" << "Telegrams"
                                    << "localField" << "devices.joinedWorlds.user.snowflake"
                                    << "foreignField" << "recipientSnowflake"
                                    << "as" << "matchingTelegrams"
                                    << finalize );
        pipeline.unwind( "$matchingTelegrams" );
        pipeline.match( document() << "matchingTelegrams.unread" << 1
                                   << finalize );
        pipeline.group( document() << "_id" << "$devices.joinedWorlds.worldID"
                                   << "numUnread" << open_document
                                       << "$count" << open_document << close_document
                                   << close_document
                                   << finalize );

        mongocxx::cursor aggregateCursor(
            collection.aggregate( pipeline )
        );

        mongocxx::cursor::iterator it( aggregateCursor.begin() );
        for( ; it != aggregateCursor.end(); ++it )
        {
            Value numUnreadValue( DocumentBuilder::unbuild( *it ) );
            numUnread[numUnreadValue["_id"]] = numUnreadValue["numUnread"];
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoTelegramLoader: Exception counting unread telegrams: " + String( e.what() ) );
        success = false;
    }

    return success;
}

} // namespace TelegramLoaders

} // namespace Agape
