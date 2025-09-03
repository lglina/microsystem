#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "World/Teleport.h"
#include "World/WorldMetadata.h"
#include "World/WorldSummary.h"
#include "Linda2WorldLoader.h"
#include "Promise.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace WorldLoaders
{

Linda2::Linda2( TupleRouter& tupleRouter,
                Timers::Factory& timerFactory ) :
  Native( "WorldLoader" ),
  m_tupleRouter( tupleRouter ),
  m_timerFactory( timerFactory ),
  m_isLoading( false ),
  m_currentJoinedWorld( 0 ),
  m_totalJoinedWorlds( -1 ),
  m_joinedWorlds( nullptr ),
  m_worldSummaries( nullptr )
{
    LOG_DEBUG( "Linda2WorldLoader: Created" );
    m_tupleRouter.registerActor( this );

    // TODO: Might need to subscribe to third-party world updates
    // to get a new user list when someone joins the world?
}

Linda2::~Linda2()
{
    m_tupleRouter.deregisterActor( this );
}

bool Linda2::create( const World::Metadata& metadata, String& reason )
{
    bool success( false );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _WorldLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _WorldCreateRequest );
    metadata.toValue( tuple[_world] );

    LOG_DEBUG( "Linda2WorldLoader: Sending WorldCreateRequest" );
    m_isLoading = true;
    m_worldCreateResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_worldCreateResponse.getFuture().get();
    m_isLoading = false;
    return success;
}

bool Linda2::join( World::Metadata& metadata, String& reason )
{
    bool success( false );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _WorldLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _WorldJoinRequest );
    metadata.toValue( tuple[_world] );

    LOG_DEBUG( "Linda2WorldLoader: Sending WorldJoinRequest" );
    m_isLoading = true;
    m_worldJoinResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_worldJoinResponse.getFuture().get();
    if( success )
    {
        metadata = m_metadata;
    }
    
    m_isLoading = false;
    return success;
}

bool Linda2::load( World::Metadata& metadata, String& reason )
{
    bool success( false );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _WorldLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _WorldLoadRequest );
    metadata.toValue( tuple[_world] );

    LOG_DEBUG( "Linda2WorldLoader: Sending WorldLoadRequest" );
    m_isLoading = true;
    m_worldLoadResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_worldLoadResponse.getFuture().get();
    if( success )
    {
        metadata = m_metadata;
    }
    
    m_isLoading = false;
    return success;
}

bool Linda2::loadJoinedWorlds( Vector< World::Metadata >& joinedWorlds, bool allDevices, String& reason )
{
    bool success( false );

    joinedWorlds.clear();

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _WorldLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _WorldLoadJoinedRequest );
    tuple[_allDevices] = allDevices ? 1 : 0;

    m_currentJoinedWorld = 0;
    m_totalJoinedWorlds = -1;

    m_joinedWorlds = &joinedWorlds;

    LOG_DEBUG( "Linda2WorldLoader: Sending WorldLoadJoinedRequest" );
    m_isLoading = true;
    m_worldLoadJoinedResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_worldLoadJoinedResponse.getFuture().get();
    m_isLoading = false;

    LOG_DEBUG( "Linda2WorldLoader: All joined worlds received" );

    return success;
}

bool Linda2::loadTeleports( Vector< World::Teleport >& teleports, bool allDevices, String& reason )
{
    bool success( false );

    teleports.clear();

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _WorldLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _WorldLoadTeleportsRequest );
    tuple[_allDevices] = allDevices ? 1 : 0;

    m_currentTeleport = 0;
    m_totalTeleports = -1;

    m_teleports = &teleports;

    LOG_DEBUG( "Linda2WorldLoader: Sending WorldLoadTeleportsRequest" );
    m_isLoading = true;
    m_worldLoadTeleportsResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_worldLoadTeleportsResponse.getFuture().get();
    m_isLoading = false;

    LOG_DEBUG( "Linda2WorldLoader: All teleports received" );

    return success;
}

bool Linda2::createTeleport( World::Teleport& teleport, String& reason )
{
    bool success( false );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _WorldLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _WorldCreateTeleportRequest );
    teleport.toValue( tuple[_teleport] );

    LOG_DEBUG( "Linda2WorldLoader: Sending WorldCreateTeleportRequest" );
    m_isLoading = true;
    m_worldCreateTeleportResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_worldCreateTeleportResponse.getFuture().get();
    m_isLoading = false;
    return success;
}

bool Linda2::deleteTeleport( World::Teleport& teleport, String& reason )
{
    bool success( false );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _WorldLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _WorldDeleteTeleportRequest );
    teleport.toValue( tuple[_teleport] );

    LOG_DEBUG( "Linda2WorldLoader: Sending WorldDeleteTeleportRequest" );
    m_isLoading = true;
    m_worldDeleteTeleportResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_worldDeleteTeleportResponse.getFuture().get();
    m_isLoading = false;
    return success;
}

bool Linda2::loadWorldSummaries( Vector< World::Summary >& worldSummaries, int from, int size, String& reason )
{
    bool success( false );

    worldSummaries.clear();

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _WorldLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _WorldLoadWorldSummariesRequest );
    tuple[_from] = from;
    tuple[_size] = size;

    m_worldSummaries = &worldSummaries;

    LOG_DEBUG( "Linda2WorldLoader: Sending WorldLoadWorldSummariesRequest" );
    m_isLoading = true;
    m_worldLoadWorldSummariesResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_worldLoadWorldSummariesResponse.getFuture().get();
    m_isLoading = false;

    LOG_DEBUG( "Linda2WorldLoader: Requested world summaries received" );

    return success;
}

bool Linda2::loadUniverseStats( World::UniverseStats& universeStats, String& reason )
{
    bool success( false );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _WorldLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _WorldLoadUniverseStatsRequest );

    m_universeStats = &universeStats;

    LOG_DEBUG( "Linda2WorldLoader: Sending WorldLoadUniverseStatsRequest" );
    m_isLoading = true;
    m_worldLoadUniverseStatsResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_worldLoadUniverseStatsResponse.getFuture().get();
    m_isLoading = false;

    LOG_DEBUG( "Linda2WorldLoader: Universe stats received" );

    return success;
}

bool Linda2::accept( Tuple& tuple )
{
    bool handled( false );

    if( ( TupleRouter::tupleType( tuple ) == _WorldCreateResponse ) && m_isLoading )
    {
        LOG_DEBUG( "Linda2WorldLoader: Received world create response" );
        m_worldCreateResponse.set( tuple[_success] );

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _WorldJoinResponse ) && m_isLoading )
    {
        LOG_DEBUG( "Linda2WorldLoader: Received world join response" );
        if( m_worldJoinResponse.set( tuple[_success] ) )
        {
            m_metadata = World::Metadata::fromValue( tuple[_world] );
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _WorldLoadResponse ) && m_isLoading )
    {
        LOG_DEBUG( "Linda2WorldLoader: Received world load response" );
        if( m_worldLoadResponse.set( tuple[_success] ) )
        {
            m_metadata = World::Metadata::fromValue( tuple[_world] );
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _WorldLoadJoinedSummary ) && m_isLoading )
    {
        LOG_DEBUG( "Linda2WorldLoader: Received joined worlds summary" );
        m_totalJoinedWorlds = tuple[_totalItems];

        if( m_currentJoinedWorld == m_totalJoinedWorlds ) // Needed where totalItems == 0!
        {
            m_worldLoadJoinedResponse.set( tuple[_success] );
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _WorldLoadJoinedResponse ) && m_isLoading )
    {
        LOG_DEBUG( "Linda2WorldLoader: Received joined world" );
        World::Metadata metadata( World::Metadata::fromValue( tuple[_world] ) );
        m_joinedWorlds->push_back( metadata );
        ++m_currentJoinedWorld;

        if( m_currentJoinedWorld == m_totalJoinedWorlds )
        {
            m_worldLoadJoinedResponse.set();
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _WorldLoadTeleportsSummary ) && m_isLoading )
    {
        LOG_DEBUG( "Linda2WorldLoader: Received teleports summary" );
        m_totalTeleports = tuple[_totalItems];

        if( m_currentTeleport == m_totalTeleports ) // Needed where totalItems == 0!
        {
            m_worldLoadTeleportsResponse.set( tuple[_success] );
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _WorldLoadTeleportsResponse ) && m_isLoading )
    {
        LOG_DEBUG( "Linda2WorldLoader: Received teleport" );
        World::Teleport teleport( World::Teleport::fromValue( tuple[_teleport] ) );
        m_teleports->push_back( teleport );
        ++m_currentTeleport;

        if( m_currentTeleport == m_totalTeleports )
        {
            m_worldLoadTeleportsResponse.set();
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _WorldCreateTeleportResponse ) && m_isLoading )
    {
        LOG_DEBUG( "Linda2WorldLoader: Received world create teleport response" );
        m_worldCreateTeleportResponse.set( tuple[_success] );

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _WorldDeleteTeleportResponse ) && m_isLoading )
    {
        LOG_DEBUG( "Linda2WorldLoader: Received world delete teleport response" );
        m_worldDeleteTeleportResponse.set( tuple[_success] );

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _WorldLoadWorldSummariesResponse ) && m_isLoading )
    {
        LOG_DEBUG( "Linda2WorldLoader: Received requested world summaries" );
        if( m_worldLoadWorldSummariesResponse.set( tuple[_success] ) )
        {
            if( tuple.hasValue( _worldSummaries ) )
            {
                Value worldSummaries( tuple[_worldSummaries] );
                ListIterator listIt( worldSummaries.listBegin() );
                for( ; listIt != worldSummaries.listEnd(); ++listIt )
                {
                    World::Summary worldSummary( World::Summary::fromValue( **listIt ) );
                    m_worldSummaries->push_back( worldSummary );
                }
            }
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _WorldLoadUniverseStatsResponse ) && m_isLoading )
    {
        LOG_DEBUG( "Linda2WorldLoader: Received universe stats" );
        if( m_worldLoadUniverseStatsResponse.set( tuple[_success] ) )
        {
            if( tuple.hasValue( _universeStats ) )
            {
                *m_universeStats = World::UniverseStats::fromValue( tuple[_universeStats] );
            }
        }

        handled = true;
    }

    return handled;
}

} // namespace WorldLoaders

} // namespace Agape
