#include "Expressions/Expression.h"
#include "Utils/LiteStream.h"
#include "CreatesStatement.h"
#include "ExecutionContext.h"
#include "Statement.h"
#include "String.h"
#include "Tuple.h"
#include "TupleRouter.h"

namespace Agape
{

namespace Carlo
{

namespace Statements
{


Creates::Creates() :
  m_type( t_unknown ),
  m_valueExpression( nullptr )
{
}

Creates::~Creates()
{
    Map< String, Expression* >::const_iterator it( m_tupleValues.begin() );
    for( ; it != m_tupleValues.end(); ++it )
    {
        delete( it->second );
    }

    delete( m_valueExpression );
}

bool Creates::eval( Value& value, ExecutionContext& executionContext )
{
    if( m_type == t_tuple )
    {
        if( executionContext.m_tuples.find( m_tupleName ) == executionContext.m_tuples.end() )
        {
            Tuple* tuple = new Tuple;
            
            bool success( true );
            Map< String, Expression* >::const_iterator it( m_tupleValues.begin() );
            for( ; success && ( it != m_tupleValues.end() ); ++it )
            {
                Value tupleValue;
                if( it->second->eval( tupleValue, executionContext ) )
                {
                    (*tuple)[it->first] = tupleValue;
                }
                else
                {
                    success = false;
                }
            }

            if( success )
            {
                TupleRouter::setTupleType( *tuple, m_tupleType );
                executionContext.m_tuples[m_tupleName] = tuple;
                return true;
            }
            else
            {
                delete( tuple ); // Prevent memory leak.
            }
        }
        else
        {
            error( ExecutionContext::errTupleAlreadyExists, executionContext );
        }

        // No return value.
    }
    else if( m_type == t_value )
    {
        if( executionContext.m_values.find( m_valueName ) == executionContext.m_values.end() )
        {
            Value* newValue = new Value;
            if( !m_valueExpression ||
                ( m_valueExpression->eval( *newValue, executionContext ) ) )
            {
                executionContext.m_values[m_valueName] = newValue;
                value = *newValue; // Return result of expression, if any, or else nothing.
                return true;
            }
            else
            {
                delete( newValue ); // Prevent memory leak.
            }
        }
        else
        {
            error( ExecutionContext::errValueAlreadyExists, executionContext );
        }
    }

    return false;
}

void Creates::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "Creates\n";
    strIndent( stream, indent );
    stream << "{\n";
    
    if( m_type == t_tuple )
    {
        strIndent( stream, indent + 4 );
        stream << "Tuple name: " << m_tupleName << "\n";
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
    else if( m_type == t_value )
    {
        strIndent( stream, indent + 4 );
        stream << "Value name: " << m_valueName << "\n";
        if( m_valueExpression )
        {
            strIndent( stream, indent + 4 );
            stream << "Value expression: " << m_valueName << "\n";
            strIndent( stream, indent + 4 );
            stream << "{\n";
            m_valueExpression->str( stream, indent + 8 );
            strIndent( stream, indent + 4 );
            stream << "}\n";
        }
    }

    strIndent( stream, indent );
    stream << "}\n";
}

} // namespace Statements

} // namespace Carlo

} // namespace Agape
