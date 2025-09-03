#include "Databases/MongoDB/MongoDB.h"
#include "Databases/MongoDB/MongoDocumentBuilder.h"
#include "Encryptors/Utils/SecureIdentifier.h"
#include "Loggers/Logger.h"
#include "World/Scene.h"
#include "World/SceneItem.h"
#include "World/WorldCoordinates.h"
#include "Authenticator.h"
#include "Collections.h"
#include "MongoSceneLoader.h"
#include "SceneLoader.h"
#include "SceneRequest.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/exception/exception.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/pipeline.hpp>

#include <cstddef>
#include <iomanip>
#include <iostream>

using namespace Agape::Databases::MongoDB;
using namespace Agape::Stratus;

using namespace bsoncxx::builder::stream;
using namespace mongocxx;

namespace Agape
{

namespace SceneLoaders
{

Mongo::Mongo( const World::Coordinates& coordinates,
              Authenticator& authenticator ) :
  SceneLoader( coordinates ),
  m_authenticator( authenticator )
{
}

bool Mongo::load( World::Scene& scene )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "MongoSceneLoader: Loading scene." );
#endif

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Scenes] );

        // Look for existing.
        bsoncxx::stdx::optional< bsoncxx::document::value > sceneDocument(
            collection.find_one(
                document() << "coordinates.worldID" << m_coordinates.m_worldID 
                        << "coordinates.x" << m_coordinates.m_x
                        << "coordinates.y" << m_coordinates.m_y
                        << finalize
            )
        );

        if( sceneDocument )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoSceneLoader: Loaded." );
            LOG_DEBUG( bsoncxx::to_json( sceneDocument->view() ).c_str() );
#endif
            Value sceneValue( DocumentBuilder::unbuild( *sceneDocument ) );
            scene = World::Scene::fromValue( sceneValue );
            return true;
        }
        else
        {
            LOG_DEBUG( "MongoSceneLoader: Scene not found. Returning empty scene." );
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoSceneLoader: Exception loading scene: " + String( e.what() ) );
    }

    return false;
}

bool Mongo::request( const Vector< SceneRequest >& requests )
{
    bool success( true );

    if( m_authenticator.writableWorld( m_coordinates.m_worldID ) )
    {
        auto it( requests.begin() );
        for( ; success && it != requests.end(); ++it )
        {
            try
            {
                success = handleRequest( *it );
            }
            catch( mongocxx::exception& e )
            {
                LOG_DEBUG( "MongoSceneLoader: Exception handling scene request: " + String( e.what() ) );
                success = false;
            }
        }
    }
    else
    {
        LOG_DEBUG( "MongoSceneLoader: Error: World not writable for user while attempting to handle requests" );
        success = false;
    }

    return success;
}

Vector< SceneRequest > Mongo::getUpdates()
{
    return Vector< SceneRequest >();
}

bool Mongo::hasSceneItemAttribute( const String& snowflake,
                                   const String& name )
{
    bool found( false );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Attributes] );

        String hashedAttributeName;
        String encryptedAttributeName;
        Encryptors::Utils::SecureIdentifier::splitIdentifier( name, hashedAttributeName, encryptedAttributeName );

        bsoncxx::stdx::optional< bsoncxx::document::value > attributesDocument(
            collection.find_one(
                document() << "coordinates.worldID" << m_coordinates.m_worldID
                           << "snowflake" << snowflake << finalize
            )
        );

        if( attributesDocument )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoSceneLoader: Loaded scene item attributes." );
            LOG_DEBUG( bsoncxx::to_json( attributesDocument->view() ).c_str() );
#endif
            Value attributesValue( DocumentBuilder::unbuild( *attributesDocument ) );
            ConstMapIterator it( attributesValue.mapBegin() );
            for( ; !found && ( it != attributesValue.mapEnd() ); ++it )
            {
                String thisHashedAttributeName;
                String thisEncryptedAttributeName;
                Encryptors::Utils::SecureIdentifier::splitIdentifier( it->first, thisHashedAttributeName, thisEncryptedAttributeName );
                found = ( thisHashedAttributeName == hashedAttributeName );
            }
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoSceneLoader: Exception loading scene item attributes: " + String( e.what() ) );
    }

    return found;
}

bool Mongo::createSceneItemAttribute( const String& snowflake,
                                      const String& name )
{
    bool success( false );

    if( m_authenticator.writableWorld( m_coordinates.m_worldID ) )
    {
        try
        {
            auto client( MongoDB::pool().acquire() );
            auto collection( ( *client )[_Agape][_Attributes] );

            Value newAttributesValue;
            newAttributesValue[name];
            m_coordinates.toValue( newAttributesValue[_coordinates] );
            bsoncxx::document::value bsonAttributes( DocumentBuilder::build( newAttributesValue ) );

#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoSceneLoader: Creating scene item attribute." );
#endif
            options::update options;
            bsoncxx::stdx::optional< mongocxx::result::update > result(
                collection.update_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID
                               << "snowflake" << snowflake << finalize,
                    document() << "$set" << bsonAttributes << finalize,
                    options.upsert( true )
                )
            );

            success = ( result && ( ( result->matched_count() == 1 ) || ( result->upserted_count() == 1 ) ) );
        }
        catch( mongocxx::exception& e )
        {
            LOG_DEBUG( "MongoSceneLoader: Exception creating scene item attribute: " + String( e.what() ) );
            success = false;
        }
    }
    else
    {
        LOG_DEBUG( "MongoSceneLoader: Error: World not writable for user while attempting to create attribute" );
        success = false;
    }

    return success;
}

bool Mongo::loadSceneItemAttribute( const String& snowflake,
                                    const String& name,
                                    Value& value )
{
    bool success( true );

    try
    {
        auto client( MongoDB::pool().acquire() );
        auto collection( ( *client )[_Agape][_Attributes] );

        String hashedAttributeName;
        String encryptedAttributeName;
        Encryptors::Utils::SecureIdentifier::splitIdentifier( name, hashedAttributeName, encryptedAttributeName );

        bsoncxx::stdx::optional< bsoncxx::document::value > attributesDocument(
            collection.find_one(
                document() << "coordinates.worldID" << m_coordinates.m_worldID
                           << "snowflake" << snowflake << finalize
            )
        );

        if( attributesDocument )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoSceneLoader: Loaded scene item attributes." );
            LOG_DEBUG( bsoncxx::to_json( attributesDocument->view() ).c_str() );
#endif
            Value attributesValue( DocumentBuilder::unbuild( *attributesDocument ) );
            ConstMapIterator it( attributesValue.mapBegin() );
            for( ; it != attributesValue.mapEnd(); ++it )
            {
                String thisHashedAttributeName;
                String thisEncryptedAttributeName;
                Encryptors::Utils::SecureIdentifier::splitIdentifier( it->first, thisHashedAttributeName, thisEncryptedAttributeName );
                if( thisHashedAttributeName == hashedAttributeName )
                {
                    value = *( it->second );
                    break;
                }
            }

            if( it == attributesValue.mapEnd() )
            {
                success = false; // Attribute not found.
            }
        }
        else
        {
            success = false;
        }
    }
    catch( mongocxx::exception& e )
    {
        LOG_DEBUG( "MongoSceneLoader: Exception loading scene item attribute: " + String( e.what() ) );
        success = false;
    }

    return success;
}

bool Mongo::saveSceneItemAttribute( const String& snowflake,
                                    const String& name,
                                    const Value& value )
{
    bool success( false );

    if( m_authenticator.writableWorld( m_coordinates.m_worldID ) )
    {
        try
        {
            auto client( MongoDB::pool().acquire() );
            auto collection( ( *client )[_Agape][_Attributes] );

            String hashedAttributeName;
            String encryptedAttributeName;
            Encryptors::Utils::SecureIdentifier::splitIdentifier( name, hashedAttributeName, encryptedAttributeName );

            // The hash part of identifier should always be the same, but the
            // encrypted part will be different each time as an arbitrary IV will be
            // used by the caller for encryption. Look up the existing identifier by
            // matching on the hash part, then use the existing identifier to
            // update the document in the database.
            String existingName;

#ifdef LOG_LOADERS
            LOG_DEBUG( "MongoSceneLoader: Save scene item attribute: Looking for existing name with hash " + hashedAttributeName );
#endif
            bsoncxx::stdx::optional< bsoncxx::document::value > attributesDocument(
                collection.find_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID
                               << "snowflake" << snowflake << finalize
                )
            );

            if( attributesDocument )
            {
                LOG_DEBUG( bsoncxx::to_json( attributesDocument->view() ).c_str() );
                Value attributesValue( DocumentBuilder::unbuild( *attributesDocument ) );
                ConstMapIterator it( attributesValue.mapBegin() );
                for( ; it != attributesValue.mapEnd(); ++it )
                {
                    String thisHashedAttributeName;
                    String thisEncryptedAttributeName;
                    Encryptors::Utils::SecureIdentifier::splitIdentifier( it->first, thisHashedAttributeName, thisEncryptedAttributeName );
                    if( thisHashedAttributeName == hashedAttributeName )
                    {
                        // Found existing attribute name.
                        existingName = it->first;
                        break;
                    }
                }

                if( !existingName.empty() )
                {
                    // Update attribute using existing name in database.
                    Value newAttributesValue;
                    newAttributesValue[existingName] = value;
                    m_coordinates.toValue( newAttributesValue[_coordinates] );
                    bsoncxx::document::value bsonAttributes( DocumentBuilder::build( newAttributesValue ) );

#ifdef LOG_LOADERS
                    LOG_DEBUG( "MongoSceneLoader: Saving scene item attribute." );
#endif
                    bsoncxx::stdx::optional< mongocxx::result::update > result(
                        collection.update_one(
                            document() << "coordinates.worldID" << m_coordinates.m_worldID
                                       << "snowflake" << snowflake << finalize,
                            document() << "$set" << bsonAttributes << finalize
                        )
                    );

                    success = ( result && ( result->matched_count() == 1 ) );
                }
                else
                {
                    LOG_DEBUG( "MongoSceneLoader: Save scene item attribute: Failed to find existing attribute name!" );
                }
            }
            else
            {
                LOG_DEBUG( "MongoSceneLoader: Failed to load attributes for scene item" );
            }
        }
        catch( mongocxx::exception& e )
        {
            LOG_DEBUG( "MongoSceneLoader: Exception saving scene item attribute: " + String( e.what() ) );
            success = false;
        }
    }
    else
    {
        LOG_DEBUG( "MongoSceneLoader: Error: World not writable for user while attempting to save attribute" );
        success = false;
    }

    return success;
}

bool Mongo::deleteSceneItemAttributes( const String& snowflake )
{
    bool success( false );

    if( m_authenticator.writableWorld( m_coordinates.m_worldID ) )
    {
        try
        {
            auto client( MongoDB::pool().acquire() );
            auto collection( ( *client )[_Agape][_Attributes] );

            bsoncxx::stdx::optional< mongocxx::result::delete_result > result(
                collection.delete_one(
                    document() << "snowflake" << snowflake << finalize
                )
            );

            success = ( result && ( result->deleted_count() == 1 ) );
        }
        catch( mongocxx::exception& e )
        {
            LOG_DEBUG( "MongoSceneLoader: Exception deleting scene item attributes: " + String( e.what() ) );
            success = false;
        }
    }
    else
    {
        LOG_DEBUG( "MongoSceneLoader: Error: World not writable for user while attempting to delete attribute" );
        success = false;
    }

    return success;
}

bool Mongo::handleRequest( const SceneRequest& request )
{
    bool success( false );

    Value sceneItemValue;
    request.m_sceneItem.toValue( sceneItemValue );
    bsoncxx::document::value bsonSceneItem( DocumentBuilder::build( sceneItemValue ) );

    auto client( MongoDB::pool().acquire() );
    auto collection( ( *client )[_Agape][_Scenes] );

    // Look for existing.
    bsoncxx::stdx::optional< bsoncxx::document::value > sceneDocument(
        collection.find_one(
            document() << "coordinates.worldID" << m_coordinates.m_worldID 
                       << "coordinates.x" << m_coordinates.m_x
                       << "coordinates.y" << m_coordinates.m_y
                       << finalize
        )
    );

    Scene currentScene;
    if( sceneDocument )
    {
        Value sceneValue( DocumentBuilder::unbuild( *sceneDocument ) );
        currentScene = World::Scene::fromValue( sceneValue );
    }

    if( request.m_sceneOperation == SceneRequest::create )
    {
        if( canCreate( currentScene, request.m_sceneItem ) )
        {
            options::update options;
            bsoncxx::stdx::optional< mongocxx::result::update > result(
                collection.update_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID
                               << "coordinates.x" << m_coordinates.m_x
                               << "coordinates.y" << m_coordinates.m_y << finalize,
                    document() << "$addToSet" << open_document << "sceneItems" << bsonSceneItem << close_document << finalize,
                    options.upsert( true ) // N.B. This will be a problem if/when Scene has more than just sceneItems.
                )
            );
            success = ( result && ( ( result->matched_count() == 1 ) || ( result->upserted_count() == 1 ) ) );
        }
    }
    else if( request.m_sceneOperation == SceneRequest::update )
    {
        if( canUpdate( currentScene, request.m_sceneItem ) )
        {
            bsoncxx::stdx::optional< mongocxx::result::update > result(
                collection.update_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID
                               << "coordinates.x" << m_coordinates.m_x
                               << "coordinates.y" << m_coordinates.m_y 
                               << "sceneItems.snowflake" << request.m_sceneItem.snowflake() << finalize,
                    document() << "$set" << open_document << "sceneItems.$" << bsonSceneItem << close_document << finalize
                )
            );
            success = ( result && ( result->matched_count() == 1 ) );
        }
    }
    else if( request.m_sceneOperation == SceneRequest::remove )
    {
        if( canRemove( currentScene, request.m_sceneItem ) )
        {
            bsoncxx::stdx::optional< mongocxx::result::update > result(
                collection.update_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID
                               << "coordinates.x" << m_coordinates.m_x
                               << "coordinates.y" << m_coordinates.m_y << finalize,
                    document() << "$pull" << open_document << "sceneItems"
                                        << open_document << "snowflake" << request.m_sceneItem.snowflake()
                                        << close_document
                                        << close_document
                                        << finalize
                )
            );
            success = ( result && ( result->modified_count() == 1 ) );
        }
    }
    else if( request.m_sceneOperation == SceneRequest::transport )
    {
        if( canUpdate( currentScene, request.m_sceneItem ) ) // canTransport?
        {
            // Remove from current scene
            bsoncxx::stdx::optional< mongocxx::result::update > result(
                collection.update_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID
                               << "coordinates.x" << m_coordinates.m_x
                               << "coordinates.y" << m_coordinates.m_y << finalize,
                    document() << "$pull" << open_document << "sceneItems"
                                        << open_document << "snowflake" << request.m_sceneItem.snowflake()
                                        << close_document
                                        << close_document
                                        << finalize
                )
            );
            success = ( result && ( result->modified_count() == 1 ) );

            // Insert into new scene
            // FIXME: Did the tuple filter check the new coordinates, if they're
            // for a different world ID, to make sure we have permission to
            // write to that world!?
            if( success )
            {
                options::update options;
                bsoncxx::stdx::optional< mongocxx::result::update > result(
                    collection.update_one(
                        document() << "coordinates.worldID" << request.m_newCoordinates.m_worldID
                                   << "coordinates.x" << request.m_newCoordinates.m_x
                                   << "coordinates.y" << request.m_newCoordinates.m_y << finalize,
                        document() << "$addToSet" << open_document << "sceneItems" << bsonSceneItem << close_document << finalize,
                        options.upsert( true ) // N.B. This will be a problem if/when Scene has more than just sceneItems.
                    )
                );
                success = ( result && ( ( result->matched_count() == 1 ) || ( result->upserted_count() == 1 ) ) );
            }
        }
    }
    else if( request.m_sceneOperation == SceneRequest::raise )
    {
        if( canUpdate( currentScene, request.m_sceneItem ) )
        {
            // Create pipeline to reorder sceneItems in array.
            mongocxx::pipeline pipeline;
            pipeline.add_fields( document() << "sceneItems"
                                            << open_document
                                              << "$concatArrays"
                                              << open_array
                                                /* All items except target item. */
                                                << open_document
                                                  << "$filter"
                                                  << open_document
                                                    << "input"
                                                    << open_document
                                                      << "$cond"
                                                      << open_document
                                                        << "if"
                                                        << open_document
                                                          << "$isArray"
                                                          << "$sceneItems"
                                                        << close_document
                                                        << "then"
                                                        << "$sceneItems"
                                                        << "else"
                                                        << open_array
                                                        << close_array
                                                      << close_document
                                                    << close_document
                                                    << "cond"
                                                    << open_document
                                                      << "$ne"
                                                      << open_array
                                                        << "$$this.snowflake"
                                                        << request.m_sceneItem.snowflake()
                                                      << close_array
                                                    << close_document
                                                  << close_document
                                                << close_document
                                                /* Finally, target item. */
                                                << open_array
                                                  << open_document
                                                    << "$first"
                                                    << open_document
                                                      << "$filter"
                                                      << open_document
                                                        << "input"
                                                        << open_document
                                                          << "$cond"
                                                          << open_document
                                                            << "if"
                                                            << open_document
                                                              << "$isArray"
                                                              << "$sceneItems"
                                                            << close_document
                                                            << "then"
                                                            << "$sceneItems"
                                                            << "else"
                                                            << open_array
                                                            << close_array
                                                          << close_document
                                                        << close_document
                                                        << "cond"
                                                        << open_document
                                                          << "$eq"
                                                          << open_array
                                                            << "$$this.snowflake"
                                                            << request.m_sceneItem.snowflake()
                                                          << close_array
                                                        << close_document
                                                      << close_document
                                                    << close_document
                                                  << close_document
                                                << close_array
                                              << close_array
                                            << close_document
                                            << finalize );

            // Apply pipeline to target scene.
            bsoncxx::stdx::optional< mongocxx::result::update > result(
                collection.update_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID
                               << "coordinates.x" << m_coordinates.m_x
                               << "coordinates.y" << m_coordinates.m_y
                               << finalize,
                    pipeline
                )
            );
            success = ( result && ( result->matched_count() == 1 ) );
        }
    }
    else if( request.m_sceneOperation == SceneRequest::lower )
    {
        if( canUpdate( currentScene, request.m_sceneItem ) )
        {
            // Create pipeline to reorder sceneItems in array.
            mongocxx::pipeline pipeline;
            pipeline.add_fields( document() << "sceneItems"
                                            << open_document
                                              << "$concatArrays"
                                              << open_array
                                                /* First, target item. */
                                                << open_array
                                                  << open_document
                                                    << "$first"
                                                    << open_document
                                                      << "$filter"
                                                      << open_document
                                                        << "input"
                                                        << open_document
                                                          << "$cond"
                                                          << open_document
                                                            << "if"
                                                            << open_document
                                                              << "$isArray"
                                                              << "$sceneItems"
                                                            << close_document
                                                            << "then"
                                                            << "$sceneItems"
                                                            << "else"
                                                            << open_array
                                                            << close_array
                                                          << close_document
                                                        << close_document
                                                        << "cond"
                                                        << open_document
                                                          << "$eq"
                                                          << open_array
                                                            << "$$this.snowflake"
                                                            << request.m_sceneItem.snowflake()
                                                          << close_array
                                                        << close_document
                                                      << close_document
                                                    << close_document
                                                  << close_document
                                                << close_array
                                                /* Finally, all items except target item. */
                                                << open_document
                                                  << "$filter"
                                                  << open_document
                                                    << "input"
                                                    << open_document
                                                      << "$cond"
                                                      << open_document
                                                        << "if"
                                                        << open_document
                                                          << "$isArray"
                                                          << "$sceneItems"
                                                        << close_document
                                                        << "then"
                                                        << "$sceneItems"
                                                        << "else"
                                                        << open_array
                                                        << close_array
                                                      << close_document
                                                    << close_document
                                                    << "cond"
                                                    << open_document
                                                      << "$ne"
                                                      << open_array
                                                        << "$$this.snowflake"
                                                        << request.m_sceneItem.snowflake()
                                                      << close_array
                                                    << close_document
                                                  << close_document
                                                << close_document
                                              << close_array
                                            << close_document
                                            << finalize );

            // Apply pipeline to target scene.
            bsoncxx::stdx::optional< mongocxx::result::update > result(
                collection.update_one(
                    document() << "coordinates.worldID" << m_coordinates.m_worldID
                               << "coordinates.x" << m_coordinates.m_x
                               << "coordinates.y" << m_coordinates.m_y
                               << finalize,
                    pipeline
                )
            );
            success = ( result && ( result->matched_count() == 1 ) );
        }
    }
    else
    {
        LOG_DEBUG( "MongoSceneLoader: Unknown scene operation" );
    }

    return success;
}

bool Mongo::canCreate( const Scene& scene, const SceneItem& sceneItem )
{
    // FIXME: STUB.
    // Move to SceneLoader as common code.
    return true;
}

bool Mongo::canUpdate( const Scene& scene, const SceneItem& sceneItem )
{
    // FIXME: STUB.
    // Move to SceneLoader as common code.
    return true;
}

bool Mongo::canRemove( const Scene& scene, const SceneItem& sceneItem )
{
    // FIXME: STUB.
    // Move to SceneLoader as common code.
    return true;
}

} // namespace SceneLoaders

} // namespace Agape
