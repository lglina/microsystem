#ifndef AGAPE_LINDA2_TUPLE_ROUTING_CRITERIA_H
#define AGAPE_LINDA2_TUPLE_ROUTING_CRITERIA_H

#include "Collections.h"
#include "String.h"
#include "Tuple.h"
#include "Value.h"

namespace Agape
{

namespace Linda2
{

class TupleRoutingCriteria
{
public:
    bool match( const Tuple& tuple ) const;

    void toTuple( Tuple& tuple ) const;
    static TupleRoutingCriteria fromTuple( const Tuple& tuple );

    String dump() const;

    Value m_types;
    Value m_destinationIDs;
    Value m_destinationActors;
    Value m_values;

private:
    bool matchType( const Tuple& tuple ) const;
    bool matchDestinationID( const Tuple& tuple ) const;
    bool matchDestinationActor( const Tuple& tuple ) const;
    bool matchValues( const Tuple& tuple ) const;
};

bool operator==( const TupleRoutingCriteria& lhs, const TupleRoutingCriteria& rhs );

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_TUPLE_ROUTING_CRITERIA_H
