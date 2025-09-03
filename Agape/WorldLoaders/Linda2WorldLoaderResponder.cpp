#include "Loggers/Logger.h"
#include "World/Teleport.h"
#include "World/UniverseStats.h"
#include "World/WorldMetadata.h"
#include "World/WorldSummary.h"
#include "WorldLoaders/Factories/WorldLoadersFactory.h"
#include "Linda2WorldLoaderResponder.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"

using namespace Agape::Linda2;
using namespace Agape::World;

namespace Agape
{

namespace WorldLoaders
{

Linda2Responder::Linda2Responder( TupleRouter& tupleRouter, WorldLoaders::Factory& worldLoaderFactory ) :
  Native( _WorldLoaderResponder ),
  m_tupleRouter( tupleRouter ),
  m_worldLoaderFactory( worldLoaderFactory )
{
    m_tupleRouter.registerActor( this );
}

Linda2Responder::~Linda2Responder()
{
    m_tupleRouter.deregisterActor( this );
}

bool Linda2Responder::accept( Tuple& tuple )
{
    bool handled( false );

    if( TupleRouter::tupleType( tuple ) ==_WorldCreateRequest )
    {
        _create( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _WorldJoinRequest )
    {
        _join( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _WorldLoadRequest )
    {
        _load( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _WorldLoadJoinedRequest )
    {
        _loadJoined( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _WorldLoadTeleportsRequest )
    {
        _loadTeleports( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _WorldCreateTeleportRequest )
    {
        _createTeleport( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _WorldDeleteTeleportRequest )
    {
        _deleteTeleport( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _WorldLoadWorldSummariesRequest )
    {
        _loadWorldSummaries( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _WorldLoadUniverseStatsRequest )
    {
        _loadUniverseStats( tuple );
        handled = true;
    }

    return handled;
}

void Linda2Responder::_create( const Tuple& tuple )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Received WorldCreateRequest" );
#endif
    WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );
    Metadata metadata( Metadata::fromValue( tuple[_world] ) );
    String reason;
    
    // FIXME: Send reason?
    Tuple response;
    TupleRouter::setSourceActor( response, _WorldLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _WorldCreateResponse );
    if( worldLoader->create( metadata, reason ) )
    {
        response[_success] = 1;
    }
    else
    {
        response[_success] = 0;
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Sending WorldCreateResponse" );
#endif
    m_tupleRouter.route( response );

    delete( worldLoader );
}

void Linda2Responder::_join( const Tuple& tuple )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Received WorldJoinRequest" );
#endif
    WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );
    Metadata metadata( Metadata::fromValue( tuple[_world] ) );
    String reason;
    
    // FIXME: Send reason?
    Tuple response;
    TupleRouter::setSourceActor( response, _WorldLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _WorldJoinResponse );

    // Note: Writability of world checked/handled in WorldLoaders::Mongo.
    if( worldLoader->join( metadata, reason ) )
    {
        response[_success] = 1;
        metadata.toValue( response[_world] );
    }
    else
    {
        response[_success] = 0;
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Sending WorldJoinResponse" );
#endif
    m_tupleRouter.route( response );

    delete( worldLoader );
}

void Linda2Responder::_load( const Tuple& tuple )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Received WorldLoadRequest" );
#endif
    WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );
    Metadata metadata( Metadata::fromValue( tuple[_world] ) );
    String reason;
    
    // FIXME: Send reason?
    Tuple response;
    TupleRouter::setSourceActor( response, _WorldLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _WorldLoadResponse );
    if( worldLoader->load( metadata, reason ) )
    {
        response[_success] = 1;
        metadata.toValue( response[_world] );
    }
    else
    {
        response[_success] = 0;
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Sending WorldLoadResponse" );
#endif
    m_tupleRouter.route( response );

    delete( worldLoader );
}

void Linda2Responder::_loadJoined( const Tuple& tuple )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Received WorldLoadJoinedRequest" );
#endif
    WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );

    Vector< Metadata > joinedWorlds;
    String reason;
    bool allDevices( (int)tuple[_allDevices] == 1 );

    Tuple response;
    TupleRouter::setSourceActor( response, _WorldLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _WorldLoadJoinedSummary );
    if( worldLoader->loadJoinedWorlds( joinedWorlds, allDevices, reason ) )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2WorldLoaderResponder: Sending WorldLoadJoinedSummary" );
#endif
        response[_totalItems] = (int)joinedWorlds.size();
        response[_success] = 1;
        m_tupleRouter.route( response );

        Vector< Metadata >::const_iterator it( joinedWorlds.begin() );
        for( ; it != joinedWorlds.end(); ++it )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "Linda2WorldLoaderResponder: Sending WorldLoadJoinedResponse" );
#endif
            Tuple worldTuple;
            TupleRouter::setSourceActor( worldTuple, _WorldLoaderResponder );
            TupleRouter::setSourceID( worldTuple, m_tupleRouter.myID() );
            TupleRouter::setDestinationID( worldTuple, TupleRouter::sourceID( tuple ) );
            TupleRouter::setTupleType( worldTuple, _WorldLoadJoinedResponse );
            it->toValue( worldTuple[_world] );
            m_tupleRouter.route( worldTuple );
        }
    }
    else
    {
        // FIXME: Send failure reason?
        response[_totalItems] = 0;
        response[_success] = 0;
        m_tupleRouter.route( response );
    }

    delete( worldLoader );
}

void Linda2Responder::_loadTeleports( const Tuple& tuple )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Received WorldLoadTeleportsRequest" );
#endif
    WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );

    Vector< Teleport > teleports;
    String reason;
    bool allDevices( (int)tuple[_allDevices] == 1 );

    Tuple response;
    TupleRouter::setSourceActor( response, _WorldLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _WorldLoadTeleportsSummary );
    if( worldLoader->loadTeleports( teleports, allDevices, reason ) )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2WorldLoaderResponder: Sending WorldLoadTeleportsSummary" );
#endif
        response[_totalItems] = (int)teleports.size();
        response[_success] = 1;
        m_tupleRouter.route( response );

        Vector< Teleport >::const_iterator it( teleports.begin() );
        for( ; it != teleports.end(); ++it )
        {
#ifdef LOG_LOADERS
            LOG_DEBUG( "Linda2WorldLoaderResponder: Sending WorldLoadTeleportsResponse" );
#endif
            Tuple worldTuple;
            TupleRouter::setSourceActor( worldTuple, _WorldLoaderResponder );
            TupleRouter::setSourceID( worldTuple, m_tupleRouter.myID() );
            TupleRouter::setDestinationID( worldTuple, TupleRouter::sourceID( tuple ) );
            TupleRouter::setTupleType( worldTuple, _WorldLoadTeleportsResponse );
            it->toValue( worldTuple[_teleport] );
            m_tupleRouter.route( worldTuple );
        }
    }
    else
    {
        // FIXME: Send failure reason?
        response[_totalItems] = 0;
        response[_success] = 0;
        m_tupleRouter.route( response );
    }

    delete( worldLoader );
}

void Linda2Responder::_createTeleport( const Tuple& tuple )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Received WorldCreateTeleportRequest" );
#endif
    WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );
    Teleport teleport( Teleport::fromValue( tuple[_teleport] ) );
    String reason;
    
    // FIXME: Send reason?
    Tuple response;
    TupleRouter::setSourceActor( response, _WorldLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _WorldCreateTeleportResponse );
    if( worldLoader->createTeleport( teleport, reason ) )
    {
        response[_success] = 1;
    }
    else
    {
        response[_success] = 0;
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Sending WorldCreateTeleportResponse" );
#endif
    m_tupleRouter.route( response );

    delete( worldLoader );
}

void Linda2Responder::_deleteTeleport( const Tuple& tuple )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Received WorldDeleteTeleportRequest" );
#endif
    WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );
    Teleport teleport( Teleport::fromValue( tuple[_teleport] ) );
    String reason;
    
    // FIXME: Send reason?
    Tuple response;
    TupleRouter::setSourceActor( response, _WorldLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _WorldDeleteTeleportResponse );
    if( worldLoader->deleteTeleport( teleport, reason ) )
    {
        response[_success] = 1;
    }
    else
    {
        response[_success] = 0;
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Sending WorldDeleteTeleportResponse" );
#endif
    m_tupleRouter.route( response );

    delete( worldLoader );
}

void Linda2Responder::_loadWorldSummaries( const Tuple& tuple )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Received WorldLoadWorldSummariesRequest" );
#endif
    WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );

    Vector< Summary > worldSummaries;
    String reason;
    int from( tuple[_from] );
    int size( tuple[_size] );

    Tuple response;
    TupleRouter::setSourceActor( response, _WorldLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _WorldLoadWorldSummariesResponse );
    if( worldLoader->loadWorldSummaries( worldSummaries, from, size, reason ) )
    {
        // FIXME: Send success/failure and reason in summary?
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2WorldLoaderResponder: Sending WorldLoadWorldIDsResponse" );
#endif
        response[_success] = 1;
        Value& worldSummariesValue( response[_worldSummaries] );
        Vector< Summary >::const_iterator worldSummariesIt( worldSummaries.begin() );
        for( ; worldSummariesIt != worldSummaries.end(); ++worldSummariesIt )
        {
            Value* worldSummaryValue = new Value;
            worldSummariesIt->toValue( *worldSummaryValue );
            worldSummariesValue.push_back( worldSummaryValue );
        }
    }
    else
    {
        response[_success] = 0;
    }

    m_tupleRouter.route( response );

    delete( worldLoader );
}

void Linda2Responder::_loadUniverseStats( const Tuple& tuple )
{
#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Received WorldLoadUniverseStatsRequest" );
#endif
    WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );
    UniverseStats universeStats;
    String reason;
    
    // FIXME: Send reason?
    Tuple response;
    TupleRouter::setSourceActor( response, _WorldLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _WorldLoadUniverseStatsResponse );
    if( worldLoader->loadUniverseStats( universeStats, reason ) )
    {
        response[_success] = 1;
        universeStats.toValue( response[_universeStats] );
    }
    else
    {
        response[_success] = 0;
    }

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2WorldLoaderResponder: Sending WorldLoadUniverseStatsResponse" );
#endif
    m_tupleRouter.route( response );

    delete( worldLoader );
}

} // namespace WorldLoaders

} // namespace Agape
