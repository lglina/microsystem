#include "Actors/Actor.h"
#include "Expressions/Expression.h"
#include "ExecutionContext.h"
#include "SendsStatement.h"
#include "Value.h"

#include "Tuple.h"
#include "TupleRouter.h"

#include "Utils/LiteStream.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace Carlo
{

namespace Statements
{

Sends::Sends( TupleRouter& tupleRouter ) :
  m_tupleRouter( tupleRouter ),
  m_type( t_unknown ),
  m_destinationActor( nullptr ),
  m_destinationID( nullptr )
{
}

Sends::~Sends()
{
    delete( m_destinationActor );
    delete( m_destinationID );

    Map< String, Expression* >::const_iterator it( m_tupleValues.begin() );
    for( ; it != m_tupleValues.end(); ++it )
    {
        delete( it->second );
    }
}

bool Sends::eval( Value& value, ExecutionContext& executionContext )
{
    bool success( true );

    if( m_type == t_tupleLiteral )
    {
        Tuple tuple;
        TupleRouter::setSourceActor( tuple, executionContext.m_currentActor->actorName() );
        TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
        TupleRouter::setTupleType( tuple, m_tupleType );

        if( m_destinationActor )
        {
            Value destinationActorValue;
            success = m_destinationActor->eval( destinationActorValue, executionContext );
            if( success )
            {
                TupleRouter::setDestinationActor( tuple, destinationActorValue );
            }
        }

        if( success && m_destinationID )
        {
            Value destinationIDValue;
            success = m_destinationID->eval( destinationIDValue, executionContext );
            if( success )
            {
                TupleRouter::setDestinationID( tuple, destinationIDValue );
            }
        }

        Map< String, Expression* >::const_iterator it( m_tupleValues.begin() );        
        for( ; success && ( it != m_tupleValues.end() ); ++it )
        {
            Value tupleValue;
            if( it->second->eval( tupleValue, executionContext ) )
            {
                tuple[it->first] = tupleValue;
            }
            else
            {
                success = false;
            }
        }

        if( success )
        {
            success = m_tupleRouter.route( tuple );
        }
    }
    else if( m_type == t_namedTuple )
    {
        if( executionContext.m_tuples.find( m_tupleName ) != executionContext.m_tuples.end() )
        {
            success = m_tupleRouter.route( *( executionContext.m_tuples[m_tupleName] ) );
        }
        else
        {
            error( ExecutionContext::errNoSuchTuple, executionContext );
            success = false;
        }
    }

    value = success ? 1 : 0; // Return value is 1 (truthy) on success.

    return success;
}

void Sends::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "SendsStatement\n";
    strIndent( stream, indent );
    stream << "{\n";
    if( m_type == t_tupleLiteral )
    {
        if( m_destinationActor )
        {
            strIndent( stream, indent + 4 );
            stream << "Destination actor:";
            strIndent( stream, indent + 4 );
            stream << "{\n";
            m_destinationActor->str( stream, indent + 8 );
            strIndent( stream, indent + 4 );
            stream << "}\n";
        }
        if( m_destinationID )
        {
            strIndent( stream, indent + 4 );
            stream << "Destination ID:";
            strIndent( stream, indent + 4 );
            stream << "{\n";
            m_destinationID->str( stream, indent + 8 );
            strIndent( stream, indent + 4 );
            stream << "}\n";
        }
        strIndent( stream, indent + 4 );
        stream << "Tuple type: " << m_tupleType << "\n";
        strIndent( stream, indent + 4 );
        stream << "Values:\n";
        strIndent( stream, indent + 4 );
        stream << "[\n";
        Map< String, Expression* >::const_iterator it( m_tupleValues.begin() );
        for( ; it != m_tupleValues.end(); ++it )
        {
            strIndent( stream, indent + 8 );
            stream << it->first << ":\n";
            it->second->str( stream, indent + 12 );
        }
        strIndent( stream, indent + 4 );
        stream << "]\n";
    }
    else if( m_type == t_namedTuple )
    {
        strIndent( stream, indent + 4 );
        stream << "Tuple: " << m_tupleName << "\n";
    }

    strIndent( stream, indent );
    stream << "}\n";
}

} // namespace Statements

} // namespace Carlo

} // namespace Agape
