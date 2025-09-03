#include "Expressions/Expression.h"
#include "Utils/LiteStream.h"
#include "Block.h"
#include "ExecutionContext.h"
#include "String.h"
#include "Tuple.h"
#include "TupleHandler.h"
#include "TupleRouter.h"

using namespace Agape::Carlo;

namespace Agape
{

namespace Linda2
{

TupleHandler::TupleHandler() :
  m_condition( nullptr ),
  m_block( nullptr ),
  m_users( 1 )
{
}

TupleHandler::~TupleHandler()
{
    delete( m_condition );
    delete( m_block );
}

bool TupleHandler::accept( Tuple& tuple,
                           ExecutionContext& executionContext )
{
    bool handled( false );

    if( ( ( TupleRouter::sourceActor( tuple ) == m_sourceActor ) || ( m_sourceActor == "" ) ) &&
        ( TupleRouter::tupleType( tuple ) == m_tupleType ) )
    {
        if( !m_tupleAlias.empty() )
        {
            executionContext.m_tuples[m_tupleAlias] = &tuple;
            executionContext.m_receivedTupleAlias = m_tupleAlias;
        }
        else
        {
            executionContext.m_tuples[m_tupleType] = &tuple;
            executionContext.m_receivedTupleAlias = m_tupleType;
        }

        Value conditionResult;
        if( !m_condition ||
            ( m_condition->eval( conditionResult, executionContext ) &&
              (int)conditionResult == 1 ) )
        {
            Value value;
            m_block->eval( value, executionContext );

            handled = true;
        }
    }

    return handled;
}

void TupleHandler::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "Handler\n";
    strIndent( stream, indent );
    stream << "{\n";
    strIndent( stream, indent + 4 );
    if( !m_sourceActor.empty() )
    {
        stream << "Source: " << m_sourceActor << "\n";
    }
    else
    {
        stream << "Source: (Any)\n";
    }
    strIndent( stream, indent + 4 );
    stream << "Tuple type: " << m_tupleType << "\n";
    m_block->str( stream, indent + 4 );

    strIndent( stream, indent );
    stream << "}\n";
}

bool TupleHandler::evalOne( Value& value, ExecutionContext& executionContext )
{
    if( m_block )
    {
        return m_block->eval( value, executionContext );
    }

    return false;
}

} // namespace Linda2

} // namespace Agape
