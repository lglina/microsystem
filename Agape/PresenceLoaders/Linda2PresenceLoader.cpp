#include "Actors/NativeActors/NativeActor.h"
#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "TupleRoutes/TupleRoute.h"
#include "World/WorldCoordinates.h"
#include "Linda2PresenceLoader.h"
#include "PresenceRequest.h"
#include "Promise.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"
#include "TupleRoutingCriteria.h"

using namespace Agape::Linda2;
using namespace Agape::World;

namespace
{
    const int maxUpdates( 8 );
} // Anonymous namespace

namespace Agape
{

namespace PresenceLoaders
{

Linda2::Linda2( const World::Coordinates& coordinates,
                bool receiveRequests,
                TupleRouter& tupleRouter,
                Timers::Factory& timerFactory ) :
  PresenceLoader( coordinates ),
  Native( _PresenceLoader ),
  m_receiveRequests( receiveRequests ),
  m_tupleRouter( tupleRouter ),
  m_timerFactory( timerFactory ),
  m_isLoading( false ),
  m_currentItem( 0 ),
  m_totalItems( -1 ),
  m_presences( nullptr ),
  m_overflowed( false )
{
    LOG_DEBUG( "Linda2PresenceLoader: Created" );
    m_tupleRouter.registerActor( this );

    if( m_receiveRequests )
    {
        m_tupleRoutingCriteria.m_types.push_back( new Value( _PresenceResponse ) );
        m_coordinates.toRoutingCriteria( m_tupleRoutingCriteria );
        m_tupleRouter.sendAddRoutingCriteriaRequest( m_tupleRoutingCriteria );

        m_updates.reserve( maxUpdates );
    }
}

Linda2::~Linda2()
{
    LOG_DEBUG( "Linda2PresenceLoader: Destroyed" );

    if( m_receiveRequests )
    {
        m_tupleRouter.sendRemoveRoutingCriteriaRequest( m_tupleRoutingCriteria );
    }

    m_tupleRouter.deregisterActor( this );
}

bool Linda2::load( Vector< World::ScenePresence >& scenePresences )
{
    bool success( true );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _PresenceLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _PresenceLoadRequest );
    m_coordinates.toValue( tuple[_coordinates] );

    scenePresences.clear();
    m_isLoading = true;
    m_currentItem = 0;
    m_totalItems = -1;

    m_presences = &scenePresences;

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2PresenceLoader: Sending PresenceLoadRequest" );
#endif
    m_presenceLoadResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_presenceLoadResponse.getFuture().get();
    m_isLoading = false;
    m_presences = nullptr;

    return success;
}

bool Linda2::loadWorld( Vector< ScenePresence >& worldPresences )
{
    bool success( true );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _PresenceLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _PresenceLoadWorldRequest );
    m_coordinates.toValue( tuple[_coordinates] );

    worldPresences.clear();
    m_isLoading = true;
    m_currentItem = 0;
    m_totalItems = -1;

    m_presences = &worldPresences;

#ifdef LOG_LOADERS
    LOG_DEBUG( "Linda2PresenceLoader: Sending PresenceLoadWorldRequest" );
#endif
    m_presenceLoadWorldResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_presenceLoadWorldResponse.getFuture().get();
    m_isLoading = false;
    m_presences = nullptr;

    return success;
}

bool Linda2::request( const Vector< PresenceRequest >& requests )
{
    bool success( true );

    for( Vector< PresenceRequest >::const_iterator it( requests.begin() ); success && ( it != requests.end() ); ++it )
    {
        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _PresenceLoader );
        TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
        TupleRouter::setTupleType( tuple, _PresenceRequest );
        it->toTuple( tuple );
        success = m_tupleRouter.route( tuple );
    }

    return success;
}

Vector< PresenceRequest > Linda2::getUpdates()
{
    // Pump handle.
    m_tupleRouter.run();
    
    Vector< PresenceRequest > updates;
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

bool Linda2::accept( Tuple& tuple )
{
    bool handled( false );

    if( ( TupleRouter::tupleType( tuple ) == _PresenceSummary ) && m_isLoading )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2PresenceLoader: Received presence summary" );
#endif
        m_totalItems = tuple[_totalItems];

        if( m_currentItem == m_totalItems )
        {
            m_presenceLoadResponse.set();
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _PresenceLoadResponse ) && m_isLoading && m_presences )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2PresenceLoader: Received presence item" );
#endif
        ScenePresence presence( ScenePresence::fromValue( tuple[_scenePresence] ) );
        m_presences->push_back( presence );
        ++m_currentItem;

        if( m_currentItem == m_totalItems )
        {
            m_presenceLoadResponse.set();
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _PresenceLoadWorldSummary ) && m_isLoading )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2PresenceLoader: Received presence load world summary" );
#endif
        m_totalItems = tuple[_totalItems];

        if( m_currentItem == m_totalItems )
        {
            m_presenceLoadWorldResponse.set();
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _PresenceLoadWorldResponse ) && m_isLoading && m_presences )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2PresenceLoader: Received presence load world item" );
#endif
        ScenePresence presence( ScenePresence::fromValue( tuple[_scenePresence] ) );
        m_presences->push_back( presence );
        ++m_currentItem;

        if( m_currentItem == m_totalItems )
        {
            m_presenceLoadWorldResponse.set();
        }

        handled = true;
    }
    else if( m_receiveRequests && ( TupleRouter::tupleType( tuple ) == _PresenceResponse ) )
    {
#ifdef LOG_LOADERS
        LOG_DEBUG( "Linda2PresenceLoader: Received presence response" );
#endif
        if( m_updates.size() < maxUpdates )
        {
            PresenceRequest request( PresenceRequest::fromTuple( tuple ) );
            m_updates.push_back( request );
        }
        else
        {
            LOG_DEBUG( "Linda2PresenceLoader: Overflow." );
        }

        handled = true;
    }

    return handled;
}

} // namespace AssetLoaders

} // namespace Agape
