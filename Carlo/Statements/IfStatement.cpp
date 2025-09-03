#include "Expressions/Expression.h"
#include "Utils/LiteStream.h"
#include "Block.h"
#include "ExecutionContext.h"
#include "IfStatement.h"
#include "Value.h"

namespace Agape
{

namespace Carlo
{

namespace Statements
{

If::If() :
  m_condition( nullptr ),
  m_block( nullptr ),
  m_elseBlock( nullptr )
{
}

If::~If()
{
    delete( m_condition );
    delete( m_block );
    delete( m_elseBlock );
}

bool If::eval( Value& value, ExecutionContext& executionContext )
{
    Value conditionResult;
    if( m_condition->eval( conditionResult, executionContext ) )
    {
        if( (int)conditionResult == 1 )
        {
            // Evaluate if block
            if( m_block->eval( value, executionContext ) )
            {
                return true;
            }
        }
        else
        {
            if( m_elseBlock )
            {
                // Evaluate else block
                if( m_elseBlock->eval( value, executionContext ) )
                {
                    return true;
                }
            }
            else
            {
                // No else block, but statement should still successfully eval().
                return true;
            }
        }
    }

    return false;
}

void If::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "IfStatement\n";
    strIndent( stream, indent );
    stream << "{\n";
    strIndent( stream, indent + 4 );
    stream << "Condition: ";
    m_condition->str( stream, indent + 4 );
    stream << "Block: ";
    m_block->str( stream, indent + 4 );
    if( m_elseBlock != nullptr )
    {
        stream << "Else block: ";
        m_elseBlock->str( stream, indent + 4 );
    }
    strIndent( stream, indent );
    stream << "}\n";
}

} // namespace Statements

} // namespace Carlo

} // namespace Agape
