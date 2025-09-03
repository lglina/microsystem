#include "Expressions/Expression.h"
#include "Utils/LiteStream.h"
#include "Block.h"
#include "ExecutionContext.h"
#include "String.h"
#include "Value.h"
#include "WhileStatement.h"

namespace Agape
{

namespace Carlo
{

namespace Statements
{

While::While() :
  m_condition( nullptr ),
  m_block( nullptr )
{
}

While::~While()
{
    delete( m_condition );
    delete( m_block );
}

bool While::eval( Value& value, ExecutionContext& executionContext )
{
    bool success( true );
    Value conditionResult;
    while( success &&
           m_condition->eval( conditionResult, executionContext ) &&
           ( (int)conditionResult == 1 ) &&
           !executionContext.m_stop )
    {
        success = m_block->eval( value, executionContext );
    }

    executionContext.m_stop = false;

    return success;
}

void While::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "WhileStatement\n";
    strIndent( stream, indent );
    stream << "{\n";
    strIndent( stream, indent + 4 );
    stream << "Condition: ";
    m_condition->str( stream, indent + 4 );
    stream << "Block: ";
    m_block->str( stream, indent + 4 );
    strIndent( stream, indent );
    stream << "}\n";
}

} // namespace Statements

} // namespace Carlo

} // namespace Agape
