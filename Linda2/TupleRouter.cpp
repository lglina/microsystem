#include "Actors/Actor.h"
#include "Timers/Factories/TimerFactory.h"
#include "TupleFilters/TupleFilter.h"
#include "TupleRoutes/TupleRoute.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Tuple.h"
#include "TupleDispatcher.h"
#include "TupleRouter.h"
#include "TupleRoutingCriteria.h"

#include "Loggers/Logger.h"

using Agape::String;

namespace Agape
{

namespace Linda2
{

TupleRouter::TupleRouter( TupleDispatcher& tupleDispatcher,
                          const String& routerName,
                          Timers::Factory& timerFactory ) :
  m_tupleDispatcher( tupleDispatcher ),
  m_routerName( routerName ),
  m_timerFactory( timerFactory ),
  m_defaultRoute( nullptr ),
  m_tupleFilter( nullptr ),
  m_routeError( false )
{
}

bool TupleRouter::route( Tuple& tuple )
{
    bool success( true );

    if( ( m_routerName != "Hydra" ) &&
        ( tupleType( tuple ) != _Tick ) &&
        ( tupleType( tuple ) != _Time ) )
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "\u001b[32mTuple:\n" + tuple.dump() + "\u001b[0m" );
#elif LOG_TUPLES_BRIEF
        LOG_DEBUG( transferDump( tuple ) );
#endif
    }

    if( ( destinationID( tuple ) != myID() ) &&
        ( destinationActor( tuple ) != _World ) &&
        ( sourceActor( tuple ) != _World ) ) // Don't route out (private?) tuples meant only for us.
    {
        /* TODO
        if( tuple.hasValue( _once ) )
        {
            // Keep selecting random routes until sendTuple returns true.
        }
        else
        {
        */

        for( Vector< TupleRoute* >::iterator it( m_tupleRoutes.begin() ); success && ( it != m_tupleRoutes.end() ); ++it )
        {
            // Each route applies its own routing rules
            bool unconditional( *it == m_defaultRoute );
            if( ( unconditional && permitOutDefault( tuple ) ) ||
                ( !unconditional && permitOut( tuple ) ) )
                
            {
                success = (*it)->sendTuple( tuple, unconditional );
#if defined(LOG_TUPLES) || defined(LOG_TUPLES_BRIEF)
                if( ( m_routerName != "Hydra" ) &&
                    ( ( *it )->name() != "Hydra" ) &&
                    ( tupleType( tuple ) != _Tick ) &&
                    ( tupleType( tuple ) != _Time ) )
                {
                    LOG_DEBUG( "\xe2\xac\x85\xef\xb8\x8f " + m_routerName + ": " + ( *it )->name() );
                }
#endif
            }
        }
    }

    // Dispatch to any local recipients. Note that the sending
    // actor may receive its own tuple back. Ignore return value as this just
    // indicates if any actor handled the tuple, and we don't care.
    m_tupleDispatcher.dispatch( tuple );

    return success;
}

void TupleRouter::addRoute( TupleRoute* route, bool defaultRoute )
{
    m_tupleRoutes.push_back( route );

    if( defaultRoute )
    {
        m_defaultRoute = route;
    }
}

void TupleRouter::removeRoute( TupleRoute* route )
{
    Vector< TupleRoute* >::iterator it( m_tupleRoutes.begin() );
    for( ; it != m_tupleRoutes.end(); ++it )
    {
        if( *it == route )
        {
            m_tupleRoutes.erase( it );

            if( m_defaultRoute == route )
            {
                m_defaultRoute = nullptr;
            }

            break;
        }
    }
}

void TupleRouter::setMyID( const String& id )
{
    m_myID = id;
}

const String& TupleRouter::myID() const
{
    return m_myID;
}

void TupleRouter::run()
{
    bool wasRouteError( m_routeError );
    m_routeError = false;
    for( Vector< TupleRoute* >::iterator it( m_tupleRoutes.begin() ); it != m_tupleRoutes.end(); ++it )
    {
        ( *it )->run();
        while( !m_routeError && ( *it )->haveIncoming() )
        {
            // Receive tuple.
            Tuple tuple;
            if( ( *it )->receiveTuple( tuple ) )
            {
                // Log tuple.
                if( TupleRouter::tupleType( tuple ) == _RoutingCriteria )
                {
#ifdef LOG_TUPLES
                    LOG_DEBUG( "\u001b[93mTuple (" + m_routerName + "):\n" + tuple.dump() + "\u001b[0m" );
                    LOG_DEBUG( "TupleRouter (" + m_routerName + "): [IN] Receiving routing criteria" );
#endif
                }
                else
                {
                    if( ( m_routerName != "Hydra" ) &&
                        ( ( *it )->name() != "Hydra" ) &&
                        ( tupleType( tuple ) != _Tick ) &&
                        ( tupleType( tuple ) != _Time ) )
                    {
#ifdef LOG_TUPLES
                        LOG_DEBUG( "\u001b[31mTuple (" + m_routerName + "):\n" + tuple.dump() + "\u001b[0m" );
#elif LOG_TUPLES_BRIEF
                        LOG_DEBUG( "\xe2\x9e\xa1\xef\xb8\x8f " + m_routerName + ": " + ( *it )->name() );
                        LOG_DEBUG( transferDump( tuple ) );
#endif
                    }
                }

                // Handle tuple.
                // Apply different filter rules depending on whether the tuple
                // is coming in via the default route.
                bool incomingDefaultRoute( *it == m_defaultRoute );
                if( ( incomingDefaultRoute && permitInDefault( tuple ) ) ||
                    ( !incomingDefaultRoute && permitIn( tuple ) ) )
                {
                    if( TupleRouter::tupleType( tuple ) == _RoutingCriteria )
                    {
                        handleRoutingRequest( tuple, *it );
                    }
                    else
                    {
                        // Forward tuple.
                        // Apply different filter rules depending on whether the
                        // tuple came in via the default route (and is now being
                        // forwarded out other routes).
                        if( ( incomingDefaultRoute && permitForwardDefault( tuple ) ) ||
                            ( !incomingDefaultRoute && permitForward( tuple ) ) )
                        {
                            // Route out to all except the incoming route.
                            // FIXME: Currently no protection against routing loops.
                            for( Vector< TupleRoute* >::iterator it2( m_tupleRoutes.begin() ); it2 != m_tupleRoutes.end(); ++it2 )
                            {
                                if( it2 != it )
                                {
                                    bool unconditional( *it2 == m_defaultRoute );
                                    // Each route applies its own routing rules
                                    ( *it2 )->sendTuple( tuple, unconditional );
#if defined(LOG_TUPLES) || defined(LOG_TUPLES_BRIEF)
                                    if( ( m_routerName != "Hydra" ) &&
                                        ( ( *it2 )->name() != "Hydra" ) &&
                                        ( tupleType( tuple ) != _Tick ) &&
                                        ( tupleType( tuple ) != _Time ) )
                                    {
                                        LOG_DEBUG( "\xe2\xac\x85\xef\xb8\x8f " + m_routerName + ": " + ( *it2 )->name() );
                                    }
#endif
                                }
                            }
                        }

                        // Dispatch to any local recipients
                        m_tupleDispatcher.dispatch( tuple );
                    }
                }
            }
            else
            {
                // Nothing received. Poll error state.
                if( ( *it )->error() )
                {
                    if( !wasRouteError )
                    {
                        LOG_DEBUG( "TupleRouter (" + m_routerName + "): Route " + ( *it )->name() + " is in error" );
                    }
                    m_routeError = true;
                }

                break;
            }
        }
    }
}

const String& TupleRouter::tupleType( const Tuple& tuple )
{
    return tuple[_type];
}

const String& TupleRouter::sourceActor( const Tuple& tuple )
{
    return tuple[_sourceActor];
}

const String& TupleRouter::destinationActor( const Tuple& tuple )
{
    return tuple[_destinationActor];
}

const String& TupleRouter::sourceID( const Tuple& tuple )
{
    return tuple[_sourceID];
}

const String& TupleRouter::destinationID( const Tuple& tuple )
{
    return tuple[_destinationID];
}

void TupleRouter::setTupleType( Tuple& tuple, const String& type )
{
    tuple[_type] = type;
}

void TupleRouter::setSourceActor( Tuple& tuple, const String& sourceActor )
{
    tuple[_sourceActor] = sourceActor;
}

void TupleRouter::setDestinationActor( Tuple& tuple, const String& destinationActor )
{
    tuple[_destinationActor] = destinationActor;
}

void TupleRouter::setSourceID( Tuple& tuple, const String& id )
{
    tuple[_sourceID] = id;
}

void TupleRouter::setDestinationID( Tuple& tuple, const String& id )
{
    tuple[_destinationID] = id;
}

void TupleRouter::registerActor( Actor* actor )
{
    m_tupleDispatcher.registerActor( actor );
}

void TupleRouter::deregisterActor( Actor* actor )
{
    m_tupleDispatcher.deregisterActor( actor );
}

void TupleRouter::registerMonitor( Actor* actor )
{
    m_tupleDispatcher.registerMonitor( actor );
}

void TupleRouter::deregisterMonitor( Actor* actor )
{
    m_tupleDispatcher.deregisterMonitor( actor );
}

void TupleRouter::sendAddRoutingCriteriaRequest( const TupleRoutingCriteria& routingCriteria )
{
    if( m_defaultRoute != nullptr )
    {
        m_defaultRoute->sendAddRoutingCriteriaRequest( routingCriteria );
    }
}

void TupleRouter::sendRemoveRoutingCriteriaRequest( const TupleRoutingCriteria& routingCriteria )
{
    if( m_defaultRoute != nullptr )
    {
        m_defaultRoute->sendRemoveRoutingCriteriaRequest( routingCriteria );
    }
}

void TupleRouter::setTupleFilter( TupleFilter* tupleFilter )
{
    m_tupleFilter = tupleFilter;
}

bool TupleRouter::routeError()
{
    return m_routeError;
}

String TupleRouter::transferDump( Tuple& tuple )
{
    LiteStream stream;

    if( TupleRouter::sourceID( tuple ) == myID() )
    {
        stream << "\x1b[91m";
    }
    else if( TupleRouter::destinationID( tuple ) == myID() )
    {
        stream << "\x1b[92m";
    }
    else
    {
        stream << "\x1b[1;97m";
    }
    stream << TupleRouter::tupleType( tuple )
           << "\x1b[0m";
    
    Tuple::ConstIterator it( tuple.begin() );
    for( ; it != tuple.end(); ++it )
    {
        const String& key( it->first );
        if( ( key != _sourceID ) &&
            ( key != _sourceActor ) &&
            ( key != _destinationID ) &&
            ( key != _destinationActor ) &&
            ( key != _type ) &&
            ( key != _data ) &&
            ( key != _sealingKey ) )
        {
            stream << " " << it->first << ":";
            const Value& value( it->second );
            switch( value.type() )
            {
            case Value::word:
            case Value::number:
                stream << it->second.toString();
                break;
            case Value::list:
                stream << "[]";
                break;
            case Value::map:
                stream << "{}";
                break;
            default:
                stream << "??";
                break;
            }
        }
        else if( key == _data )
        {
            stream << " " << it->first << ": (data)";
        }
        else if( key == _sealingKey )
        {
            stream << " " << it->first << ": ***";
        }
    }

    return stream.str();
}

void TupleRouter::handleRoutingRequest( const Tuple& tuple, TupleRoute* route )
{
    TupleRoutingCriteria routingCriteria( TupleRoutingCriteria::fromTuple( tuple ) );
    if( tuple[_action] == String(_add) )
    {
        route->addRoutingCriteria( routingCriteria );
    }
    else if( tuple[_action] == String(_remove) )
    {
        route->removeRoutingCriteria( routingCriteria );
    }

    // Propagate to all other routes.
    // FIXME: No routing loop checking!
    for( Vector< TupleRoute* >::iterator it( m_tupleRoutes.begin() ); it != m_tupleRoutes.end(); ++it )
    {
        if( ( *it == m_defaultRoute ) && ( *it != route ) )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleRouter (" + m_routerName + "): Propagating routing criteria to default route" );
#endif
            if( tuple[_action] == String(_add) )
            {
                ( *it )->sendAddRoutingCriteriaRequest( routingCriteria );
            }
            else if( tuple[_action] == String(_remove) )
            {
                ( *it )->sendRemoveRoutingCriteriaRequest( routingCriteria );
            }
        }
    }
}

bool TupleRouter::permitIn( const Tuple& tuple )
{
    return( !m_tupleFilter || m_tupleFilter->permitIn( tuple ) );
}

bool TupleRouter::permitInDefault( const Tuple& tuple )
{
    return( !m_tupleFilter || m_tupleFilter->permitInDefault( tuple ) );
}

bool TupleRouter::permitForward( const Tuple& tuple )
{
    return( !m_tupleFilter || m_tupleFilter->permitForward( tuple ) );
}

bool TupleRouter::permitForwardDefault( const Tuple& tuple )
{
    return( !m_tupleFilter || m_tupleFilter->permitForwardDefault( tuple ) );
}

bool TupleRouter::permitOut( const Tuple& tuple )
{
    return( !m_tupleFilter || m_tupleFilter->permitOut( tuple ) );
}

bool TupleRouter::permitOutDefault( const Tuple& tuple )
{
    return( !m_tupleFilter || m_tupleFilter->permitOutDefault( tuple ) );
}

} // namespace Linda2

} // namespace Agape
