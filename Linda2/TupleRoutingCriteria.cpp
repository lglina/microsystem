#include "Loggers/Logger.h"
#include "Collections.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"
#include "TupleRoutingCriteria.h"

using Agape::String;

namespace Agape
{

namespace Linda2
{

bool TupleRoutingCriteria::match( const Tuple& tuple ) const
{
#ifdef LOG_TUPLES
    LOG_DEBUG( dump() );
#endif

    bool match( ( matchType( tuple ) && matchValues( tuple ) ) ||
                ( matchDestinationID( tuple ) && matchValues( tuple ) ) ||
                ( matchDestinationActor( tuple ) && matchValues( tuple ) ) );

    if( match )
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "Routing criteria matched for this route" );
#endif
    }
    else
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "Routing criteria NOT matched for this route" );
#endif
    }

    return match;
}

void TupleRoutingCriteria::toTuple( Tuple& tuple ) const
{
    TupleRouter::setTupleType( tuple, _RoutingCriteria );
    tuple[_types] = m_types;
    tuple[_destinationIDs] = m_destinationIDs;
    tuple[_destinationActors] = m_destinationActors;
    tuple[_values] = m_values;
}

TupleRoutingCriteria TupleRoutingCriteria::fromTuple( const Tuple& tuple )
{
    TupleRoutingCriteria tupleRoutingCriteria;
    tupleRoutingCriteria.m_types = tuple[_types];
    tupleRoutingCriteria.m_destinationIDs = tuple[_destinationIDs];
    tupleRoutingCriteria.m_destinationActors = tuple[_destinationActors];
    tupleRoutingCriteria.m_values = tuple[_values];
    return tupleRoutingCriteria;
}

String TupleRoutingCriteria::dump() const
{
    String s;
    s += "\u001b[35m";
    s += "CRITERIA: Types:\n" + m_types.dump();
    s += "CRITERIA: Destination IDs:\n" + m_destinationIDs.dump();
    s += "CRITERIA: Destination actors:\n" + m_destinationActors.dump();
    s += "CRITERIA: Values:\n" + m_values.dump();
    s += "\u001b[0m";
    return s;
}

bool TupleRoutingCriteria::matchType( const Tuple& tuple ) const
{
    const String& type( TupleRouter::tupleType( tuple ) );
    return( m_types.hasValue( type ) );
}

bool TupleRoutingCriteria::matchDestinationID( const Tuple& tuple ) const
{
    const String& destinationID( TupleRouter::destinationID( tuple ) );
    return( m_destinationIDs.hasValue( destinationID ) );
}

bool TupleRoutingCriteria::matchDestinationActor( const Tuple& tuple ) const
{
    const String& destinationActor( TupleRouter::destinationActor( tuple ) );
    return( m_destinationActors.hasValue( destinationActor ) );
}

bool TupleRoutingCriteria::matchValues( const Tuple& tuple ) const
{
    ConstMapIterator it( m_values.mapBegin() );
    for( ; it != m_values.mapEnd(); ++it )
    {
        if( ( !tuple.hasValue( it->first ) ) ||
            ( tuple[it->first] != *( it->second ) ) )
        {
            return false;
        }
    }

    return true;
}

bool operator==( const TupleRoutingCriteria& lhs, const TupleRoutingCriteria& rhs )
{
    return( ( lhs.m_types == rhs.m_types ) &&
            ( lhs.m_destinationIDs == rhs.m_destinationIDs ) &&
            ( lhs.m_destinationActors == rhs.m_destinationActors ) &&
            ( lhs.m_values == rhs.m_values ) );
}

} // namespace Linda2

} // namespace Agape
