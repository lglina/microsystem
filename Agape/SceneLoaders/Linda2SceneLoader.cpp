#include "Actors/NativeActors/NativeActor.h"
#include "Encryptors/Utils/SecureIdentifier.h"
#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "World/Scene.h"
#include "World/SceneItem.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "Linda2SceneLoader.h"
#include "Promise.h"
#include "SceneLoader.h"
#include "SceneRequest.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"
#include "TupleRoutingCriteria.h"

using namespace Agape::Encryptors::Utils;
using namespace Agape::Linda2;
using namespace Agape::World;

namespace
{
    const int maxUpdates( 8 );
    //const int cacheTimeout( 5000 ); // ms
    const int cacheTimeout( 0 ); // ms
} // Anonymous namespace

namespace Agape
{

namespace SceneLoaders
{

// TODO: We could re-work Compositor so that scene loading is asynchronous (i.e.
// we send a request for scene data, then update the compositor piecemeal as
// each SceneItem tuple arrives). This would require covering off on some
// Compositor edge cases, such as a user attempting to edit a scene while it's
// still loading. For now, scene loading is blocking/synchronous.

Linda2::Linda2( const Coordinates& coordinates,
                bool receiveRequests,
                TupleRouter& tupleRouter,
                Timers::Factory& timerFactory,
                bool encrypted ) :
  SceneLoader( coordinates ),
  Native( _SceneLoader ),
  m_receiveRequests( receiveRequests ),
  m_tupleRouter( tupleRouter ),
  m_timerFactory( timerFactory ),
  m_encrypted( encrypted ),
  m_isLoading( false ),
  m_currentItem( 0 ),
  m_totalItems( -1 ),
  m_currentScene( nullptr ),
  m_isSavingAttribute( false ),
  m_isDeletingAttributes( false ),
  m_overflowed( false )
{
    LOG_DEBUG( "Linda2SceneLoader: Created" );
    m_tupleRouter.registerActor( this );

    if( m_receiveRequests )
    {
        m_sceneLoadResponseRoutingCriteria.m_types.push_back( new Value( _SceneResponse ) );
        m_coordinates.toRoutingCriteria( m_sceneLoadResponseRoutingCriteria );
        m_tupleRouter.sendAddRoutingCriteriaRequest( m_sceneLoadResponseRoutingCriteria );

        // Receive transport responses where our current coordinates are the
        // destination of the transport.
        m_sceneLoadResponseTransportRoutingCriteria.m_types.push_back( new Value( _SceneResponse ) );
        m_coordinates.toRoutingCriteria( m_sceneLoadResponseTransportRoutingCriteria, _newCoordinates );
        m_tupleRouter.sendAddRoutingCriteriaRequest( m_sceneLoadResponseTransportRoutingCriteria );

        m_sceneItemSaveAttributeRoutingCriteria.m_types.push_back( new Value( _SceneItemSaveAttributeResponse ) );
        m_coordinates.toRoutingCriteria( m_sceneItemSaveAttributeRoutingCriteria );
        m_tupleRouter.sendAddRoutingCriteriaRequest( m_sceneItemSaveAttributeRoutingCriteria );

        m_sceneItemDeleteAttributesRoutingCriteria.m_types.push_back( new Value( _SceneItemDeleteAttributesResponse ) );
        m_coordinates.toRoutingCriteria( m_sceneItemDeleteAttributesRoutingCriteria );
        m_tupleRouter.sendAddRoutingCriteriaRequest( m_sceneItemDeleteAttributesRoutingCriteria );

        m_updates.reserve( maxUpdates );

        m_invalidateCachedAssetRoutingCriteria.m_types.push_back( new Value( _InvalidateCachedAsset ) );
        m_coordinates.toRoutingCriteria( m_invalidateCachedAssetRoutingCriteria, _coordinates, true ); // true = all scenes in world
        m_tupleRouter.sendAddRoutingCriteriaRequest( m_invalidateCachedAssetRoutingCriteria );

        m_invalidatedAssets.reserve( maxUpdates );
    }
}

Linda2::~Linda2()
{
    LOG_DEBUG( "Linda2SceneLoader: Destroyed" );

    if( m_receiveRequests )
    {
        m_tupleRouter.sendRemoveRoutingCriteriaRequest( m_sceneLoadResponseRoutingCriteria );
        m_tupleRouter.sendRemoveRoutingCriteriaRequest( m_sceneLoadResponseTransportRoutingCriteria );
        m_tupleRouter.sendRemoveRoutingCriteriaRequest( m_sceneItemSaveAttributeRoutingCriteria );
        m_tupleRouter.sendRemoveRoutingCriteriaRequest( m_sceneItemDeleteAttributesRoutingCriteria );
        m_tupleRouter.sendRemoveRoutingCriteriaRequest( m_invalidateCachedAssetRoutingCriteria );
    }

    m_tupleRouter.deregisterActor( this );

    purgeCache();
}

bool Linda2::load( Scene& scene )
{
    bool success( true );

    scene.m_sceneItems.clear();

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _SceneLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _SceneLoadRequest );
    m_coordinates.toValue( tuple[_coordinates] );

    m_isLoading = true;
    m_currentItem = 0;
    m_totalItems = -1;

    m_currentScene = &scene;

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2SceneLoader: Sending sceneLoadRequest" );
#endif
    m_sceneLoadResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_sceneLoadResponse.getFuture().get();

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2SceneLoader: All scene items received" );
#endif
    // Ignore any new incoming load responses, to allow other loaders to
    // operate in parallel, e.g. to create MiniMap.
    // FIXME: This is now used in SceneLoader, AssetLoader and PresenceLoader
    // and feels nasty, as it requires loaders to be blocking to work properly.
    // We should use a request ID system instead to distinguish
    // request/response pairs from each other.
    m_isLoading = false;
    m_currentScene = nullptr;
    // Scene now loaded.

    return success;
}

void Linda2::save( const Scene& scene )
{
}

bool Linda2::request( const Vector< SceneRequest >& requests )
{
    bool success( true );

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2SceneLoader: Sending scene requests" );
#endif
    for( Vector< SceneRequest >::const_iterator it( requests.begin() ); success && ( it != requests.end() ); ++it )
    {
        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _SceneLoader );
        TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
        TupleRouter::setTupleType( tuple, _SceneRequest );
        it->toTuple( tuple );
        success = m_tupleRouter.route( tuple );
    }

    return success;
}

Vector< SceneRequest > Linda2::getUpdates()
{
    // Pump handle.
    m_tupleRouter.run();

    Vector< SceneRequest > updates;
    updates = m_updates;
    m_updates.clear();
    return updates;
}

bool Linda2::overflowed()
{
    if( m_overflowed )
    {
        m_overflowed = false;
        return true;
    }

    return false;
}

bool Linda2::hasSceneItemAttribute( const String& snowflake,
                                    const String& name )
{
    // Rather than having this as an explicit RPC to the server, for efficiency,
    // perform a load to bring the current value (if any) into the cache, as
    // the next request is usually LoadSceneItemAttribute.
    Value value;
    return loadSceneItemAttribute( snowflake, name, value );
}

bool Linda2::createSceneItemAttribute( const String& snowflake,
                                       const String& name )
{
    // Cache locally even if the remote request eventually fails, so that we
    // can change our local state in non-writable worlds.

    // FIXME: We will still return a failure to the caller, which will throw
    // a runtime error in user Carlo code, potentially breaking games. If we
    // know here that the world is non-writable (or if the server gives that as
    // a failure reason), do we just pretend the operation succeeded?
    saveToCache( snowflake, name, Value() );

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2SceneLoader: Sending scene item create attribute request" );
#endif
    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _SceneLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _SceneItemCreateAttributeRequest );
    m_coordinates.toValue( tuple[_coordinates] );
    tuple[_snowflake] = snowflake;
    tuple[_name] = name;
    
    m_sceneItemCreateAttributeResponse = Promise( &m_tupleRouter, &m_timerFactory );
    if( m_tupleRouter.route( tuple ) ) return m_sceneItemCreateAttributeResponse.getFuture().get();
    return false;
}

bool Linda2::loadSceneItemAttribute( const String& snowflake,
                                     const String& name,
                                     Value& value )
{
    if( loadFromCache( snowflake, name, value ) )
    {
        return true;
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2SceneLoader: Sending scene item load attribute request" );
#endif
    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _SceneLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _SceneItemLoadAttributeRequest );
    m_coordinates.toValue( tuple[_coordinates] );
    tuple[_snowflake] = snowflake;
    tuple[_name] = name;

    m_sceneItemLoadAttributeResponse = Promise( &m_tupleRouter, &m_timerFactory );
    if( m_tupleRouter.route( tuple ) ) return m_sceneItemLoadAttributeResponse.getFuture().get( value );
    return false;
}

bool Linda2::saveSceneItemAttribute( const String& snowflake,
                                     const String& name,
                                     const Value& value )
{
    // Cache locally even if the remote request eventually fails, so that we
    // can change our local state in non-writable worlds.
    saveToCache( snowflake, name, value );

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2SceneLoader: Sending scene item save attribute request" );
#endif
    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _SceneLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _SceneItemSaveAttributeRequest );
    m_coordinates.toValue( tuple[_coordinates] );
    tuple[_snowflake] = snowflake;
    tuple[_name] = name;
    tuple[_attribute] = value;

    m_isSavingAttribute = true;
    m_saveDeleteSnowflake = snowflake;
    m_saveAttributeName = name;
    m_sceneItemSaveAttributeResponse = Promise( &m_tupleRouter, &m_timerFactory );
    if( m_tupleRouter.route( tuple ) ) return m_sceneItemSaveAttributeResponse.getFuture().get();
    return false;
}

bool Linda2::deleteSceneItemAttributes( const String& snowflake )
{
    // Delete from cache locally even if the remote request eventually fails,
    // so that we can change our local state in non-writable worlds.
    deleteFromCache( snowflake );

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2SceneLoader: Sending scene item delete attribute request" );
#endif
    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _SceneLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _SceneItemDeleteAttributesRequest );
    m_coordinates.toValue( tuple[_coordinates] );
    tuple[_snowflake] = snowflake;
    
    m_isDeletingAttributes = true;
    m_saveDeleteSnowflake = snowflake;
    m_sceneItemDeleteAttributesResponse = Promise( &m_tupleRouter, &m_timerFactory );
    if( m_tupleRouter.route( tuple ) ) return m_sceneItemDeleteAttributesResponse.getFuture().get();
    return false;
}

void Linda2::invalidateCachedAsset( const struct InvalidatedAsset& invalidatedAsset )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2SceneLoader: Sending cached asset invalidation notification" );
#endif
    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _SceneLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _InvalidateCachedAsset );
    m_coordinates.toValue( tuple[_coordinates], true ); // True = write worldID only.
    tuple[_assetName] = invalidatedAsset.m_name;
    tuple[_assetType] = invalidatedAsset.m_type;
    tuple[_originatorID] = invalidatedAsset.m_originatorID;
    m_tupleRouter.route( tuple );
}

Vector< struct SceneLoader::InvalidatedAsset > Linda2::getInvalidatedAssets()
{
    // Pump handle.
    m_tupleRouter.run();
    
    Vector< struct InvalidatedAsset > invalidatedAssets;
    invalidatedAssets = m_invalidatedAssets;
    m_invalidatedAssets.clear();
    return invalidatedAssets;
}

bool Linda2::accept( Agape::Linda2::Tuple& tuple )
{
    bool handled( false );

    if( ( TupleRouter::tupleType( tuple ) == _SceneSummary ) && m_isLoading )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2SceneLoader: Received scene summary" );
#endif
        m_totalItems = tuple[_totalItems];

        if( m_currentItem == m_totalItems ) // Needed where totalItems == 0!
        {
            m_sceneLoadResponse.set();
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _SceneLoadResponse ) && m_isLoading && m_currentScene )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2SceneLoader: Received scene item" );
#endif
        if( m_currentScene->m_sceneItems.size() < m_currentScene->maxItems() )
        {
            SceneItem sceneItem( World::SceneItem::fromValue( tuple[_sceneItem] ) );
            m_currentScene->m_sceneItems.push_back( sceneItem );
        }
        else
        {
            LOG_DEBUG( "Linda2SceneLoader: Warning: Scene too large. Items dropped." );
        }
        ++m_currentItem;

        if( m_currentItem == m_totalItems )
        {
            m_sceneLoadResponse.set();
        }

        handled = true;
    }
    else if( m_receiveRequests && ( TupleRouter::tupleType( tuple ) == _SceneResponse ) )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2SceneLoader: Received scene request response" );
#endif
        if( m_updates.size() < maxUpdates )
        {
            SceneRequest request( SceneRequest::fromTuple( tuple ) );
            m_updates.push_back( request );
        }
        else
        {
            LOG_DEBUG( "Linda2SceneLoader: Overflow." );
            m_overflowed = true;
        }

        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _SceneItemCreateAttributeResponse )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2SceneLoader: Received scene item create attribute response" );
#endif
        m_sceneItemCreateAttributeResponse.set( tuple[_success] );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _SceneItemLoadAttributeResponse )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2SceneLoader: Received scene item load attribute response" );
#endif
        m_sceneItemLoadAttributeResponse.set( tuple[_success], tuple[_attribute] );
        handled = true;

        // Here (and below) we will have already saved the value to the cache
        // if we initiated the action, however we're also snooping others'
        // attribute changes, so we still need to do this here to cache
        // those changes.
        if( (int)tuple[_success] == 1 )
        {
            saveToCache( tuple[_snowflake], tuple[_name], tuple[_attribute] );
        }
    }
    else if( TupleRouter::tupleType( tuple ) == _SceneItemSaveAttributeResponse )
    {
        if( m_isSavingAttribute &&
            ( tuple[_snowflake] == m_saveDeleteSnowflake ) &&
            ( tuple[_name] == m_saveAttributeName ) ) // Item and attribute match what we're waiting for.
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "Linda2SceneLoader: Received scene item save attribute response" );
#endif
            m_sceneItemSaveAttributeResponse.set( tuple[_success] );
            m_isSavingAttribute = false;
            handled = true;
        }

        if( (int)tuple[_success] == 1 )
        {
            saveToCache( tuple[_snowflake], tuple[_name], tuple[_attribute] );
        }
    }
    else if( TupleRouter::tupleType( tuple ) == _SceneItemDeleteAttributesResponse )
    {
        if( m_isDeletingAttributes &&
            ( tuple[_snowflake] == m_saveDeleteSnowflake ) ) // Item and attribute match what we're waiting for.
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "Linda2SceneLoader: Received scene item delete attributes response" );
#endif
            m_sceneItemDeleteAttributesResponse.set( tuple[_success] );
            m_isDeletingAttributes = false;
            handled = true;
        }

        if( (int)tuple[_success] == 1 )
        {
            deleteFromCache( tuple[_snowflake] );
        }
    }
    else if( m_receiveRequests && ( TupleRouter::tupleType( tuple ) == _InvalidateCachedAsset ) )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2SceneLoader: Received cached asset invalidation notification" );
#endif
        if( m_invalidatedAssets.size() < maxUpdates )
        {
            struct InvalidatedAsset invalidatedAsset;
            invalidatedAsset.m_name = tuple[_assetName];
            invalidatedAsset.m_type = tuple[_assetType];
            invalidatedAsset.m_originatorID = tuple[_originatorID];
            m_invalidatedAssets.push_back( invalidatedAsset );
        }
        else
        {
            LOG_DEBUG( "Linda2SceneLoader: Asset invalidations overflow." );
        }

        handled = true;
    }

    return handled;
}

void Linda2::saveToCache( const String& snowflake, const String& name, const Value& value )
{
    String constName( name );
    if( m_encrypted )
    {
        String hash;
        String cipherText;
        SecureIdentifier::splitIdentifier( name, hash, cipherText );
        constName = hash;
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "Trying to save attribute " + constName + " for item with snowflake " + snowflake + " to cache" );
#endif

    Map< String, Value* >::const_iterator it( m_cachedAttributes.find( snowflake ) );
    if( it != m_cachedAttributes.end() )
    {
        // Add to/overwrite existing cached attributes for this item.
        Value& cachedAttributes( *( it->second ) );
        cachedAttributes[constName] = value;
#ifdef LOG_LOADERS
        LOG_DEBUG( "Added/overwrote attribute " + constName + " in cache for item with snowflake " + snowflake );
#endif
    }
    else
    {
        // Create new cached attributes for item and save this attribute.
        Value* newAttributes = new Value();
        ( *newAttributes )[constName] = value;
        m_cachedAttributes[snowflake] = newAttributes;
#ifdef LOG_LOADERS
        LOG_DEBUG( "Created new attribute cache for item with snowflake " + snowflake );
#endif
    }
}

bool Linda2::loadFromCache( const String& snowflake, const String& name, Value& value )
{
    // See if cached attribute exists for this item.
    
    // TODO: A few different options for caching aside from this implementation:
    // * Have Compositor keep the current state of attributes and Compositor
    //   polls here (for AttributeRequest objects?) as for scene updates.
    // * A separate "CachingSceneLoader" that passes through to the underlying
    //   SceneLoader.
    // Not sure if these have merit/benefits, so leaving it this way for now.

    // FIXME: We need to know if we're dealing with encrypted or plain names
    // here, as if encrypted we only want to store and match on the constant
    // hashed part. This feels like it breaks encapsulation, though, as we're
    // havig knowledge of what some other layer (EncryptedSceneLoader) is
    // doing. In any case, we want to move this to CachingSceneLoader at some
    // point, as we may want to cache scene data to local flash as well
    // as attributes.

    String constName( name );
    if( m_encrypted )
    {
        String hash;
        String cipherText;
        SecureIdentifier::splitIdentifier( name, hash, cipherText );
        constName = hash;
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "Trying to load cached attribute " + constName + " for item with snowflake " + snowflake );
#endif

    Map< String, Value* >::const_iterator it( m_cachedAttributes.find( snowflake ) );
    if( it != m_cachedAttributes.end() )
    {
        Value& cachedAttributes( *( it->second ) );
        if( cachedAttributes.hasValue( constName ) )
        {
            value = cachedAttributes[constName];
#ifdef LOG_LOADERS
            LOG_DEBUG( "Attribute found in cache" );
#endif
            return true;
        }
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "Attribute not found in cache" );
#endif

    return false;
}

void Linda2::deleteFromCache( const String& snowflake )
{
    Map< String, Value* >::const_iterator it( m_cachedAttributes.find( snowflake ) );
    if( it != m_cachedAttributes.end() )
    {
        delete( it->second );
        m_cachedAttributes.erase( it );
    }
}

void Linda2::purgeCache()
{
    Map< String, Value* >::iterator it( m_cachedAttributes.begin() );
    for( ; it != m_cachedAttributes.end(); ++it )
    {
        delete( it->second );
    }
    m_cachedAttributes.clear();
}

} // namespace SceneLoaders

} // namespace Agape
