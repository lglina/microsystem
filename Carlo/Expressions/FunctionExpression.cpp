#include "Actors/Actor.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Utils/Tokeniser.h"
#include "Collections.h"
#include "ExecutionContext.h"
#include "Expression.h"
#include "FunctionDispatcher.h"
#include "FunctionExpression.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace Carlo
{

namespace Expressions
{

Function::Function( FunctionDispatcher& functionDispatcher ) :
  m_functionDispatcher( functionDispatcher )
{
}

Function::~Function()
{
    Map< String, Expression* >::const_iterator it( m_argumentExpressions.begin() );
    for( ; it != m_argumentExpressions.end(); ++it )
    {
        delete( it->second );
    }
}

bool Function::eval( Value& value, ExecutionContext& executionContext )
{
    Tokeniser tokeniser( m_name, '.' );
    String actorName( tokeniser.token() );
    String functionName;
    if( !tokeniser.atEnd() )
    {
        functionName = tokeniser.token();
    }
    else
    {
        functionName = actorName;
        actorName.clear();
    }

    Map< String, Value* > arguments;
    Map< String, Expression* >::const_iterator argumentExpressionsIt( m_argumentExpressions.begin() );
    bool success( true );
    for( ; success && ( argumentExpressionsIt != m_argumentExpressions.end() ); ++argumentExpressionsIt )
    {
        Value* argument = new Value;
        if( argumentExpressionsIt->second->eval( *argument, executionContext ) )
        {
            arguments[argumentExpressionsIt->first] = argument;
#ifdef LOG_CARLO_INTERP
            LiteStream stream;
            stream << "Argument " << argumentExpressionsIt->first << ": " << arguments[argumentExpressionsIt->first]->dump( 0 );
            LOG_DEBUG( stream.str() );
#endif
        }
        else
        {
            delete( argument );
            success = false;
        }
    }

    if( success )
    {
#ifdef LOG_CARLO_INTERP
        LOG_DEBUG( "Function: Dispatching function" );
#endif
        success = m_functionDispatcher.dispatch( value,
                                                 actorName,
                                                 functionName,
                                                 arguments,
                                                 executionContext.m_currentActor->actorName() );

        if( !success )
        {
            error( ExecutionContext::errNoSuchFunction, executionContext );
        }
    }

    Map< String, Value* >::const_iterator argumentValuesIt( arguments.begin() );
    for( ; argumentValuesIt != arguments.end(); ++argumentValuesIt )
    {
        delete( argumentValuesIt->second );
    }

    return success;
}

void Function::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "FunctionExpression\n";
    strIndent( stream, indent );
    stream << "{\n";
    strIndent( stream, indent + 4 );
    stream << "Name: " << m_name << "\n";

    strIndent( stream, indent + 4 );
    stream << "Arguments:\n";
    strIndent( stream, indent + 4 );
    stream << "[\n";
    Map< String, Expression* >::const_iterator it( m_argumentExpressions.begin() );
    for( ; it != m_argumentExpressions.end(); ++it )
    {
        strIndent( stream, indent + 8 );
        stream << it->first << ":\n";
        it->second->str( stream, indent + 12 );
    }
    strIndent( stream, indent + 4 );
    stream << "]\n";

    strIndent( stream, indent );
    stream << "}\n";
}

} // namespace Expressions

} // namespace Carlo

} // namespace Agape
