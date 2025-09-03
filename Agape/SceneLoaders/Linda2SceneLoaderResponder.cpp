#include "SceneLoaders/Factories/SceneLoadersFactory.h"
#include "Utils/LiteStream.h"
#include "World/Scene.h"
#include "World/SceneItem.h"
#include "World/WorldCoordinates.h"
#include "Linda2SceneLoaderResponder.h"
#include "SceneLoader.h"
#include "SceneRequest.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"

#include "Loggers/Logger.h"

using namespace Agape::Linda2;
using namespace Agape::World;

namespace Agape
{

namespace SceneLoaders
{

Linda2Responder::Linda2Responder( TupleRouter& tupleRouter, SceneLoaders::Factory& sceneLoaderFactory ) :
  Actors::Native( _SceneLoaderResponder ),
  m_tupleRouter( tupleRouter ),
  m_sceneLoaderFactory( sceneLoaderFactory )
{
    // FIXME: Move this to NativeActor.
    m_tupleRouter.registerActor( this );
}

Linda2Responder::~Linda2Responder()
{
    m_tupleRouter.deregisterActor( this );
}

bool Linda2Responder::accept( Tuple& tuple )
{
    bool handled( false );

    if( TupleRouter::tupleType( tuple ) == _SceneLoadRequest )
    {
        loadScene( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _SceneRequest )
    {
        handleRequest( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _SceneItemCreateAttributeRequest )
    {
        handleCreateAttributeRequest( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _SceneItemLoadAttributeRequest )
    {
        handleLoadAttributeRequest( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _SceneItemSaveAttributeRequest )
    {
        handleSaveAttributeRequest( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _SceneItemDeleteAttributesRequest )
    {
        handleDeleteAttributesRequest( tuple );
        handled = true;
    }

    return handled;
}

void Linda2Responder::reset()
{
    // FIXME: Where is this called?
}

void Linda2Responder::loadScene( const Tuple& tuple )
{
    Coordinates coordinates( Coordinates::fromValue( tuple[_coordinates] ) );
    SceneLoader* sceneLoader( m_sceneLoaderFactory.makeLoader( coordinates ) );

#ifdef LOG_LOADERS
    LiteStream stream;
    stream << "Linda2SceneLoaderResponder: Received SceneLoadRequest for "
           << coordinates.m_worldID << " " << coordinates.m_x << "," << coordinates.m_y;
    LOG_DEBUG( stream.str() );
#endif

    Scene scene;
    sceneLoader->load( scene );

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2SceneLoaderResponder: Sending SceneSummary" );
#endif
    Tuple response;
    TupleRouter::setSourceActor( response, _SceneLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _SceneSummary );
    response[_totalItems] = (int)scene.m_sceneItems.size();
    m_tupleRouter.route( response );

    Vector< SceneItem >::const_iterator it( scene.m_sceneItems.begin() );
    for( ; it != scene.m_sceneItems.end(); ++it )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2SceneLoaderResponder: Sending SceneLoadResponse" );
#endif
        Tuple sceneItemTuple;
        TupleRouter::setSourceActor( sceneItemTuple, _SceneLoaderResponder );
        TupleRouter::setSourceID( sceneItemTuple, m_tupleRouter.myID() );
        TupleRouter::setDestinationID( sceneItemTuple, TupleRouter::sourceID( tuple ) );
        TupleRouter::setTupleType( sceneItemTuple, _SceneLoadResponse );
        it->toValue( sceneItemTuple[_sceneItem] );
        m_tupleRouter.route( sceneItemTuple );
    }

    delete( sceneLoader );
}

void Linda2Responder::handleRequest( const Tuple& tuple )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2SceneLoaderResponder: Handling SceneRequest" );
#endif
    SceneRequest request( SceneRequest::fromTuple( tuple ) );

    Vector< SceneRequest > requests;
    requests.push_back( request );

    SceneLoader* sceneLoader( m_sceneLoaderFactory.makeLoader( request.m_coordinates ) );
    sceneLoader->request( requests );
    delete( sceneLoader );

    // FIXME: ACK/NAK?
    Tuple response;
    TupleRouter::setSourceActor( response, _SceneLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _SceneResponse );
    request.m_originatorID = TupleRouter::sourceID( tuple );
    request.toTuple( response );
    request.m_coordinates.toValue( response[_coordinates] );
    m_tupleRouter.route( response );
}

void Linda2Responder::handleCreateAttributeRequest( const Tuple& tuple )
{
    Tuple response;
    TupleRouter::setSourceActor( response, _SceneLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _SceneItemCreateAttributeResponse );

    Coordinates coordinates( Coordinates::fromValue( tuple[_coordinates] ) );
    SceneLoader* sceneLoader( m_sceneLoaderFactory.makeLoader( coordinates ) );
    
    coordinates.toValue( response[_coordinates] );
    response[_snowflake] = tuple[_snowflake];
    response[_name] = tuple[_name];
    response[_success] = sceneLoader->createSceneItemAttribute( tuple[_snowflake], tuple[_name] ) ? 1 : 0;
    
    delete( sceneLoader );

    m_tupleRouter.route( response );
}

void Linda2Responder::handleLoadAttributeRequest( const Tuple& tuple )
{
    Tuple response;
    TupleRouter::setSourceActor( response, _SceneLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _SceneItemLoadAttributeResponse );

    Coordinates coordinates( Coordinates::fromValue( tuple[_coordinates] ) );
    SceneLoader* sceneLoader( m_sceneLoaderFactory.makeLoader( coordinates ) );
    Value value;
    
    coordinates.toValue( response[_coordinates] );
    response[_snowflake] = tuple[_snowflake];
    response[_name] = tuple[_name];
    response[_success] = sceneLoader->loadSceneItemAttribute( tuple[_snowflake], tuple[_name], value );
    response[_attribute] = value;
    
    delete( sceneLoader );

    m_tupleRouter.route( response );
}

void Linda2Responder::handleSaveAttributeRequest( const Tuple& tuple )
{
    Tuple response;
    TupleRouter::setSourceActor( response, _SceneLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _SceneItemSaveAttributeResponse );

    Coordinates coordinates( Coordinates::fromValue( tuple[_coordinates] ) );
    SceneLoader* sceneLoader( m_sceneLoaderFactory.makeLoader( coordinates ) );
    
    coordinates.toValue( response[_coordinates] );
    response[_snowflake] = tuple[_snowflake];
    response[_name] = tuple[_name];
    response[_success] = sceneLoader->saveSceneItemAttribute( tuple[_snowflake], tuple[_name], tuple[_attribute] );
    response[_attribute] = tuple[_attribute];
    
    delete( sceneLoader );

    m_tupleRouter.route( response );
}

void Linda2Responder::handleDeleteAttributesRequest( const Tuple& tuple )
{
    Tuple response;
    TupleRouter::setSourceActor( response, _SceneLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _SceneItemDeleteAttributesResponse );

    Coordinates coordinates( Coordinates::fromValue( tuple[_coordinates] ) );
    SceneLoader* sceneLoader( m_sceneLoaderFactory.makeLoader( coordinates ) );
    
    coordinates.toValue( response[_coordinates] );
    response[_snowflake] = tuple[_snowflake];
    response[_name] = tuple[_name];
    response[_success] = sceneLoader->deleteSceneItemAttributes( tuple[_snowflake] );
    
    delete( sceneLoader );

    m_tupleRouter.route( response );
}

} // namespace SceneLoaders

} // namespace Agape
