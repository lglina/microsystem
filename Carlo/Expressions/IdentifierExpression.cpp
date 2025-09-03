#include "Actors/Actor.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Utils/Tokeniser.h"
#include "Collections.h"
#include "ExecutionContext.h"
#include "Expression.h"
#include "FunctionDispatcher.h"
#include "IdentifierExpression.h"
#include "StringConstants.h"
#include "String.h"
#include "Tuple.h"
#include "Value.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace Carlo
{

namespace Expressions
{

Identifier::Identifier( FunctionDispatcher& functionDispatcher ) :
  m_functionDispatcher( functionDispatcher ),
  m_quotedString( false )
{
}

bool Identifier::evalAssignable( Value*& value, ExecutionContext& executionContext )
{
    if( m_quotedString )
    {
        // Parser should never allow a quoted string to be used as an lvalue,
        // but check here to be safe.
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
        LOG_DEBUG( "EvalAssignable: Attempted to use quoted string literal as lvalue" );
#endif
        return false;
    }

    if( _evalAssignable( value, executionContext ) )
    {
        return true;
    }

    error( ExecutionContext::errNoSuchTupleOrValue, executionContext );
    return false;
}

bool Identifier::eval( Value& value, ExecutionContext& executionContext )
{
    // Return an rvalue, i.e. where the caller owns the memory and we
    // copy into the caller's Value.
    bool found( false );

    if( !m_string.empty() )
    {
        if( m_quotedString )
        {
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
            LOG_DEBUG( "Eval: IdentifierExpression: Quoted string literal" );
#endif
            value = m_string;
            found = true;
        }

        if( !found && ( m_string == _nothing ) )
        {
            found = true;
        }

        if( !found )
        {
            // Regular function?
            Tokeniser tokeniser( m_string, '.' );
            String actorName( parseToken( tokeniser.token(), executionContext ) );
            String functionName( parseToken( tokeniser.token(), executionContext ) );

            if( !actorName.empty() && !functionName.empty() )
            {
                Map< String, Value* > arguments;
                if( m_functionDispatcher.dispatch( value,
                                                   actorName,
                                                   functionName,
                                                   arguments,
                                                   executionContext.m_currentActor->actorName() ) )
                {
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
                    LOG_DEBUG( "Eval: IdentifierExpression: Found regular function" );
#endif
                    found = true;

                    while( !tokeniser.atEnd() )
                    {
                        String token( parseToken( tokeniser.token(), executionContext ) );
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
                        LOG_DEBUG( "EvalAssignable: IdentifierExpression: Sub-value:" + token );
#endif
                        if( !token.empty() )
                        {
                            Value innerValue( *getValueValue( value, token ) );
                            value = innerValue;
                        }
                        else
                        {
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
                            LOG_DEBUG( "EvalAssignable: Unable to parse token in nested value chain." );
#endif
                            found = false;
                        }
                    }
                }
            }
        }

        if( !found && ( m_string.find( '.' ) == String::npos ) )
        {
            // Inbuilt function (e.g. sin)?
            Map< String, Value* > arguments;
            if( m_functionDispatcher.dispatch( value,
                                               String(),
                                               m_string,
                                               arguments,
                                               executionContext.m_currentActor->actorName() ) )
            {
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
                LOG_DEBUG( "Eval: IdentifierExpression: Found inbuilt function" );
#endif
                found = true;
            }
        }

        if( !found )
        {
            // If we can find an lvalue, we can return it as an rvalue.
            Value* assignable( nullptr );
            if( _evalAssignable( assignable, executionContext ) )
            {
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
                LOG_DEBUG( "Eval: IdentifierExpression: Using lvalue as rvalue" );
#endif
                value = *assignable;
                found = true;
            }
        }

        if( !found && ( m_string.find( '.' ) == String::npos ) )
        {
            // Return literal, but only if the identifier has no dots
            // (if it does, assume we have a malformed path identifier
            // or function name, and fail).
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
            LOG_DEBUG( "Eval: IdentifierExpression: String literal" );
#endif
            value = m_string;
            found = true;
        }
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
        else
        {
            LOG_DEBUG( "Eval: IdentifierExpression: Assume malformed path or function" );
        }
#endif
    }
    else
    {
        // Float literal.
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
        LOG_DEBUG( "Eval: IdentifierExpression: Float literal" );
#endif
        value = m_float;
        found = true;
    }

    if( !found )
    {
        error( ExecutionContext::errNoSuchTupleValueOrFunction, executionContext );
    }

    return found;
}

void Identifier::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "IdentifierExpression\n";
    strIndent( stream, indent );
    stream << "{\n";
    strIndent( stream, indent + 4 );
    stream << "String: " << m_string << "\n";
    strIndent( stream, indent + 4 );
    stream << "Float: " << m_float << "\n";
    strIndent( stream, indent );
    stream << "}\n";
}

bool Identifier::_evalAssignable( Value*& value, ExecutionContext& executionContext )
{
    // Return an lvalue, i.e. where ExecutionContext owns the memory and
    // the caller receives a pointer to a modifiable Value in ExecutionContext.
    
    bool found( false );

#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
    LOG_DEBUG( "EvalAssignable: IdentifierExpression: " + m_string );
#endif

    Tokeniser tokeniser( m_string, '.' );
    String token( parseToken( tokeniser.token(), executionContext ) );
    String resolvedName = token;

    Value* persistableValue( nullptr );
    String persistableBaseName;

#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
    if( token.empty() )
    {
        LOG_DEBUG( "EvalAssignable: IdentifierExpression: Unable to parse first token." );
    }
#endif

    // Value?
    if( !token.empty() )
    {
        value = getNamedValue( executionContext, token );
        if( value )
        {
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
            LOG_DEBUG( "EvalAssignable: IdentifierExpression: Found named value" );
#endif
            found = true;
        }
    }

    // Tuple?
    if( !found && !token.empty() && !tokeniser.atEnd() )
    {
        Tuple* tuple( getNamedTuple( executionContext, token ) );
        if( tuple )
        {
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
            LOG_DEBUG( "EvalAssignable: IdentifierExpression: Found named tuple" );
#endif
            token = parseToken( tokeniser.token(), executionContext );
            if( !token.empty() )
            {
                resolvedName += "." + token;
                value = getTupleValue( *tuple, token );
                found = true;
            }
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
            else
            {
                LOG_DEBUG( "EvalAssignable: IdentifierExpression: Unable to parse token after tuple name." );
            }
#endif
        }
    }
    
    // Persistable value?
    if( !found && !token.empty() && !tokeniser.atEnd() )
    {
        persistableValue = new Value;
        String valueName( parseToken( tokeniser.token(), executionContext ) );
        if( !valueName.empty() && 
            m_functionDispatcher.getPersistableValue( *persistableValue,
                                                      token,
                                                      valueName,
                                                      executionContext.m_currentActor->actorName() ) )
        {
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
            LOG_DEBUG( "EvalAssignable: IdentifierExpression: Found persistable value" );
#endif
            resolvedName += "." + valueName;
            persistableBaseName = resolvedName;
            value = persistableValue;
            found = true;
        }
        else
        {
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
            if( valueName.empty() )
            {
                LOG_DEBUG( "EvalAssignable: IdentifierExpression: Unable to parse token after actor name" );
            }
#endif
            delete( persistableValue );
            persistableValue = nullptr;
        }
    }

    // Descend if needed until we find the inner value (after last period).
    // FIXME: This will create inner values if they don't exist, which isn't
    // what we want when isRValue...
    if( found )
    {
        while( !tokeniser.atEnd() )
        {
            token = parseToken( tokeniser.token(), executionContext );
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
            LOG_DEBUG( "EvalAssignable: IdentifierExpression: Sub-value:" + token );
#endif
            if( !token.empty() )
            {
                resolvedName += "." + token;
                value = getValueValue( *value, token );

                // If descending into a persistable value, ensure the nested
                // value has a reference to the base value so e.g.
                // MakesStatement can later call save() and we can find the
                // base ValueLoader.
                if( persistableValue ) value->setParent( persistableValue );
            }
            else
            {
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
                LOG_DEBUG( "EvalAssignable: Unable to parse token in nested value chain." );
#endif
                found = false;
            }
        }

        if( persistableValue )
        {
            // If we created a new value here (as a result of a function call),
            // pass ownership over to ExecutionContext.
            if( executionContext.m_values.find( persistableBaseName ) != executionContext.m_values.end() )
            {
#if defined(LOG_CARLO_INTERP) || defined(LOG_CARLO_INTERP_IDENT)
                LOG_DEBUG( "EvalAssignable: IdentifierExpression: Purge persistable value" );
#endif
                // Delete any previous copy of the value.
                // The destructor will persist any changes previously made.
                delete( executionContext.m_values[persistableBaseName] );
            }
            executionContext.m_values[persistableBaseName] = persistableValue;
        }
    }
    
    return found;
}

Tuple* Identifier::getNamedTuple( ExecutionContext& executionContext, const String& name )
{
    Map< String, Tuple* >::iterator it( executionContext.m_tuples.find( name ) );
    if( it != executionContext.m_tuples.end() )
    {
        return executionContext.m_tuples[name];
    }

    return nullptr;
}

Value* Identifier::getNamedValue( ExecutionContext& executionContext, const String& name )
{
    Map< String, Value* >::iterator it( executionContext.m_values.find( name ) );
    if( it != executionContext.m_values.end() )
    {
        return executionContext.m_values[name];
    }

    return nullptr;
}

Value* Identifier::getTupleValue( Tuple& tuple, const String& name )
{
    // Create if not exist.
    return &tuple[name];
}

Value* Identifier::getValueValue( Value& value, const String& name )
{
    // Create if not exist.
    return &value[name];
}

String Identifier::parseToken( const String& token, ExecutionContext& executionContext )
{
    if( ( token.length() > 1 ) &&
        ( token[0] == '@' ) )
    {
        String valueName( token.substr( 1 ) );
        Value* value( getNamedValue( executionContext, valueName ) );
        if( value &&
            ( value->type() == Value::word ) )
        {
            return *value;
        }
        else
        {
            return String();
        }
    }

    return token;
}

} // namespace Expressions

} // namespace Carlo

} // namespace Agape
