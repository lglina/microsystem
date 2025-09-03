#include "Actors/NativeActors/NativeActor.h"
#include "Loggers/Logger.h"
#include "PresenceLoaders/Factories/PresenceLoadersFactory.h"
#include "PresenceLoaders/PresenceLoader.h"
#include "Utils/LiteStream.h"
#include "World/ScenePresence.h"
#include "World/WorldCoordinates.h"
#include "Linda2PresenceLoaderResponder.h"
#include "Collections.h"
#include "PresenceRequest.h"
#include "Promise.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"

using namespace Agape::Linda2;
using namespace Agape::World;

namespace Agape
{

namespace PresenceLoaders
{

Linda2Responder::Linda2Responder( TupleRouter& tupleRouter, PresenceLoaders::Factory& presenceLoaderFactory ) :
  Actors::Native( _PresenceLoaderResponder ),
  m_tupleRouter( tupleRouter ),
  m_presenceLoaderFactory( presenceLoaderFactory )
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

    if( TupleRouter::tupleType( tuple ) == _PresenceLoadRequest )
    {
        loadPresences( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _PresenceLoadWorldRequest )
    {
        loadWorldPresences( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _PresenceRequest )
    {
        handleRequest( tuple );
        handled = true;
    }

    return handled;
}

void Linda2Responder::reset()
{
    // FIXME: Where is this called?
}

void Linda2Responder::forceDepart()
{
    // Ensure a departure event is sent on destruction.
    // FIXME: This may result in duplicate departure events, if the client
    // sent a departure event before disconnecting. Is this bad?
    Map< String, PresenceRequest >::const_iterator it( m_latestPresenceRequests.begin() );
    for( ; it != m_latestPresenceRequests.end(); ++it )
    {
        PresenceRequest departRequest( it->second );
        departRequest.m_presenceOperation = PresenceRequest::depart;
        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _PresenceLoaderResponder );
        TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
        TupleRouter::setTupleType( tuple, _PresenceResponse );
        departRequest.toTuple( tuple );
        m_tupleRouter.route( tuple );

        Vector< PresenceRequest > requests;
        requests.push_back( departRequest );

        PresenceLoader* presenceLoader( m_presenceLoaderFactory.makeLoader( departRequest.m_coordinates ) );
        presenceLoader->request( requests );
        delete( presenceLoader );
    }
}

void Linda2Responder::loadPresences( const Tuple& tuple )
{
    Coordinates coordinates( Coordinates::fromValue( tuple[_coordinates] ) );

#ifdef LOG_LOADERS
    LiteStream stream;
    stream << "Linda2PresenceLoaderResponder: Received PresenceLoadRequest for "
           << coordinates.m_worldID << " " << coordinates.m_x << "," << coordinates.m_y;
    LOG_DEBUG( stream.str() );
#endif

    PresenceLoader* presenceLoader( m_presenceLoaderFactory.makeLoader( coordinates ) );
    Vector< ScenePresence > presences;
    presenceLoader->load( presences ); // FIXME: Should send error back in summary tuple here?
    delete( presenceLoader );

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2PresenceLoaderResponder: Sending PresenceSummary" );
#endif
    Tuple response;
    TupleRouter::setSourceActor( response, _PresenceLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _PresenceSummary );
    response[_totalItems] = (int)presences.size();
    m_tupleRouter.route( response );

    Vector< ScenePresence >::const_iterator it( presences.begin() );
    for( ; it != presences.end(); ++it )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2PresenceLoaderResponder: Sending PresenceLoadResponse" );
#endif
        Tuple presenceTuple;
        TupleRouter::setSourceActor( presenceTuple, _PresenceLoaderResponder );
        TupleRouter::setSourceID( presenceTuple, m_tupleRouter.myID() );
        TupleRouter::setDestinationID( presenceTuple, TupleRouter::sourceID( tuple ) );
        TupleRouter::setTupleType( presenceTuple, _PresenceLoadResponse );
        it->toValue( presenceTuple[_scenePresence] );
        m_tupleRouter.route( presenceTuple );
    }
}

void Linda2Responder::loadWorldPresences( const Tuple& tuple )
{
    Coordinates coordinates( Coordinates::fromValue( tuple[_coordinates] ) );

#ifdef LOG_LOADERS
    LiteStream stream;
    stream << "Linda2PresenceLoaderResponder: Received PresenceLoadWorldRequest for "
           << coordinates.m_worldID;
    LOG_DEBUG( stream.str() );
#endif

    PresenceLoader* presenceLoader( m_presenceLoaderFactory.makeLoader( coordinates ) );
    Vector< ScenePresence > presences;
    presenceLoader->loadWorld( presences ); // FIXME: Should send error back in summary tuple here?
    delete( presenceLoader );

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2PresenceLoaderResponder: Sending PresenceLoadWorldSummary" );
#endif
    Tuple response;
    TupleRouter::setSourceActor( response, _PresenceLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _PresenceLoadWorldSummary );
    response[_totalItems] = (int)presences.size();
    m_tupleRouter.route( response );

    Vector< ScenePresence >::const_iterator it( presences.begin() );
    for( ; it != presences.end(); ++it )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2PresenceLoaderResponder: Sending PresenceLoadWorldResponse" );
#endif
        Tuple presenceTuple;
        TupleRouter::setSourceActor( presenceTuple, _PresenceLoaderResponder );
        TupleRouter::setSourceID( presenceTuple, m_tupleRouter.myID() );
        TupleRouter::setDestinationID( presenceTuple, TupleRouter::sourceID( tuple ) );
        TupleRouter::setTupleType( presenceTuple, _PresenceLoadWorldResponse );
        it->toValue( presenceTuple[_scenePresence] );
        m_tupleRouter.route( presenceTuple );
    }
}

void Linda2Responder::handleRequest( const Tuple& tuple )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2PresenceLoaderResponder: Handling PresenceRequest" );
#endif
    PresenceRequest request( PresenceRequest::fromTuple( tuple ) );

    // Save latest request for each user so we can generate
    // guaranteed departure events on destruction.
    const String& snowflake( request.m_scenePresence.m_user.m_snowflake );
    m_latestPresenceRequests[snowflake] = request;

    Vector< PresenceRequest > requests;
    requests.push_back( request );
    
    // FIXME: Can probably persist the loader, until coordinates change on
    // future request.
    PresenceLoader* presenceLoader( m_presenceLoaderFactory.makeLoader( request.m_coordinates ) );
    presenceLoader->request( requests );
    delete( presenceLoader );

    // FIXME: ACK/NAK?
    Tuple response;
    TupleRouter::setSourceActor( response, _PresenceLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _PresenceResponse );
    request.m_originatorID = TupleRouter::sourceID( tuple );
    request.toTuple( response );
    request.m_coordinates.toValue( response[_coordinates] );
    m_tupleRouter.route( response );
}

} // namespace PresenceLoaders

} // namespace Agape
