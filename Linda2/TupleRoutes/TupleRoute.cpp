#include "Utils/LiteStream.h"
#include "ANSITerminal.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRoute.h"
#include "TupleRouter.h"
#include "TupleRoutingCriteria.h"

#include "Loggers/Logger.h"

using Agape::String;

namespace Agape
{

namespace Linda2
{

TupleRoute::TupleRoute( const String& routeName ) :
  m_routeName( routeName )
{
}

bool TupleRoute::sendTuple( const Tuple& tuple, bool unconditional )
{
    bool success( true );

    if( !error() )
    {
        if( unconditional || canRoute( tuple ) )
        {
#ifdef LOG_TUPLES
            if( !unconditional )
            {
                LOG_DEBUG( "TupleRoute (" + m_routeName + "): Sending tuple to route on routing criteria" );
            }
            else
            {
                LOG_DEBUG( "TupleRoute (" + m_routeName + "): Sending tuple to route unconditionally" );
            }
#endif

            success = _sendTuple( tuple );
        } // else success true, although we didn't send.
    }
    else
    {
        success = false;
    }

    return success;
}

void TupleRoute::sendAddRoutingCriteriaRequest( const TupleRoutingCriteria& routingCriteria )
{
#if defined(LOG_TUPLES) || defined(LOG_TUPLES_BRIEF)
    LOG_DEBUG( "TupleRoute (" + m_routeName + "): Sending routing criteria add request" );
    LOG_DEBUG( routingCriteria.dump() );
#endif
    Tuple tuple;
    routingCriteria.toTuple( tuple );
    tuple[_action] = _add;
    sendTuple( tuple, true );
}

void TupleRoute::sendRemoveRoutingCriteriaRequest( const TupleRoutingCriteria& routingCriteria )
{
#if defined(LOG_TUPLES) || defined(LOG_TUPLES_BRIEF)
    LOG_DEBUG( "TupleRoute (" + m_routeName + "): Sending routing criteria remove request" );
    LOG_DEBUG( routingCriteria.dump() );
#endif
    Tuple tuple;
    routingCriteria.toTuple( tuple );
    tuple[_action] = _remove;
    sendTuple( tuple, true );
}

void TupleRoute::addRoutingCriteria( const TupleRoutingCriteria& routingCriteria )
{
#if defined(LOG_TUPLES) || defined(LOG_TUPLES_BRIEF)
    LOG_DEBUG( "TupleRoute (" + m_routeName + "): Receiving and applying routing criteria" );
#endif
    m_tupleRoutingCriteria.push_back( routingCriteria );
}

void TupleRoute::removeRoutingCriteria( const TupleRoutingCriteria& routingCriteria )
{
#if defined(LOG_TUPLES) || defined(LOG_TUPLES_BRIEF)
    LOG_DEBUG( "TupleRoute (" + m_routeName + "): Removing routing criteria" );
#endif
    bool wasErased( false );
    Vector< TupleRoutingCriteria >::iterator it( m_tupleRoutingCriteria.begin() );
    for( ; it != m_tupleRoutingCriteria.end(); ++it )
    {
        if( *it == routingCriteria )
        {
#if defined(LOG_TUPLES) || defined(LOG_TUPLES_BRIEF)
            LOG_DEBUG( "Removed" );
#endif
            m_tupleRoutingCriteria.erase( it );
            wasErased = true;
            break;
        }
    }

    if( !wasErased )
    {
#if defined(LOG_TUPLES) || defined(LOG_TUPLES_BRIEF)
        LOG_DEBUG( "Couldn't find criteria to remove !?" );
#endif
    }
}

const String& TupleRoute::name()
{
    return m_routeName;
}

bool TupleRoute::canRoute( const Tuple& tuple ) const
{
    Vector< TupleRoutingCriteria >::const_iterator it( m_tupleRoutingCriteria.begin() );
    for( ; it != m_tupleRoutingCriteria.end(); ++it )
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "TupleRoute (" + m_routeName + "): Evaluating criteria" );
#endif
        if( it->match( tuple ) )
        {
            return true;
        }
    }

    if( TupleRouter::tupleType( tuple ) == "RoutingCriteria" )
    {
        return true;
    }

    return false;
}

} // namespace Linda2

} // namespace Agape
