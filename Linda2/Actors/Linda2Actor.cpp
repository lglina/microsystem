#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Collections.h"
#include "ExecutionContext.h"
#include "Linda2Actor.h"
#include "String.h"
#include "Tuple.h"
#include "TupleHandler.h"
#include "TupleRouter.h"
#include "TupleRoutingCriteria.h"

//#include <iostream>

using namespace Agape::Carlo;

namespace Agape
{

namespace Linda2
{

namespace Actors
{

Linda2Actor::Linda2Actor( const String& name, TupleRouter& tupleRouter ) :
  m_name( name ),
  m_tupleRouter( tupleRouter )
{
}

Linda2Actor::~Linda2Actor()
{
    LOG_DEBUG( "Destructing actor " + m_name );
    Vector< TupleHandler* >::iterator it( m_tupleHandlers.begin() );
    for( ; it != m_tupleHandlers.end(); ++it )
    {
        TupleHandler* tupleHandler( *it );
        --( tupleHandler->m_users );

        LiteStream stream;
        stream << "Tuple handler for " << tupleHandler->m_tupleType
                << " now has " << tupleHandler->m_users << " users.";
        LOG_DEBUG( stream.str() );

        if( tupleHandler->m_users == 0 )
        {
            // Delete when reference count zero.
            LOG_DEBUG( "No remaining referents. Deleting." );
            delete( *it );
        }
    }

    m_tupleRouter.deregisterActor( this );
}

void Linda2Actor::doRegister()
{
    m_tupleRouter.registerActor( this );

    // FIXME: We currently don't remove any routing criteria added here when
    // this actor is destructed, as we don't know if any other actors need
    // them. We should have a mechanism to centrally coordinate the needs of
    // all actors.
    
    // FIXME: For TupleHandlers ("receives" clauses) we create routing criteria
    // based on type only and not values.

    // Register for tuples for this actor.
    {
    TupleRoutingCriteria criteria;
    criteria.m_destinationActors = m_name;
    m_tupleRouter.sendAddRoutingCriteriaRequest( criteria );
    }

    // Register for tuples of all types referenced in receives clauses.
    // FIXME: We should probably add the current worldID (using Coordinates) to
    // the routing criteria below, otherwise we're subscribing to all tuples
    // with this actor name and/or type from ALL worlds (even though the server
    // will block us from receiving them if we haven't joined those worlds). The
    // best way to achieve this would be to modify the Carlo "receives" syntax
    // to allow program writers to get the current coordinates from the World
    // NativeActor, and we pick that up here from TupleHandler to put it into
    // the routing criteria, instead of/in addition to eval()'ing the
    // TupleHandler condition in its accept() function for incoming tuples.
    Vector< TupleHandler* >::const_iterator it( m_tupleHandlers.begin() );
    for( ; it != m_tupleHandlers.end(); ++it )
    {
        TupleRoutingCriteria criteria;
        criteria.m_types.push_back( new Value( (*it)->m_tupleType ) );
        m_tupleRouter.sendAddRoutingCriteriaRequest( criteria );
    }
}

bool Linda2Actor::accept( Tuple& tuple )
{
    //std::cout << "Actor " << this << " evaluating tuple" << std::endl;

    bool handled( false );

    ExecutionContext executionContext;
    executionContext.m_currentActor = this;

    // FIXME: Optimise so we don't have to iterate over every handler
    // (e.g. Map based on tuple type?)
    Vector< TupleHandler* >::iterator it( m_tupleHandlers.begin() );
    for( ; it != m_tupleHandlers.end(); ++it )
    {
        if( ( *it )->accept( tuple, executionContext ) )
        {
            handled = true;
            (*it)->m_runtimeErrors = executionContext.m_runtimeErrors;
            break;
        }
    }

    return handled;
}

String Linda2Actor::actorName() const
{
    return m_name;
}

void Linda2Actor::rename( const String& name )
{
    m_name = name;
}

void Linda2Actor::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "Actor\n";
    strIndent( stream, indent );
    stream << "{\n";
    Vector< TupleHandler* >::const_iterator it( m_tupleHandlers.begin() );
    for( ; it != m_tupleHandlers.end(); ++it )
    {
        ( *it )->str( stream, indent + 4 );
    }
    strIndent( stream, indent );
    stream << "}\n";
}

const Vector< ExecutionContext::RuntimeError > Linda2Actor::runtimeErrors() const
{
    Vector< ExecutionContext::RuntimeError > runtimeErrors;

    Vector< TupleHandler* >::const_iterator handlersIt( m_tupleHandlers.begin() );
    for( ; handlersIt != m_tupleHandlers.end(); ++handlersIt )
    {
        const Vector< ExecutionContext::RuntimeError >& thisHandlerErrors( (*handlersIt)->m_runtimeErrors );
        Vector< ExecutionContext::RuntimeError >::const_iterator errorsIt( thisHandlerErrors.begin() );
        for( ; errorsIt != thisHandlerErrors.end(); ++errorsIt )
        {
            runtimeErrors.push_back( *errorsIt );
        }
    }

    return runtimeErrors;
}

bool Linda2Actor::evalOne( Value& value, ExecutionContext& executionContext )
{
    if( !m_tupleHandlers.empty() )
    {
        executionContext.m_currentActor = this;
        return ( *( m_tupleHandlers.begin() ) )->evalOne( value, executionContext );
    }

    return false;
}

} // namespace Actors

} // namespace Linda2

} // namespace Agape
