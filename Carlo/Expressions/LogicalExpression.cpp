#include "Utils/LiteStream.h"
#include "ExecutionContext.h"
#include "Expression.h"
#include "LogicalExpression.h"
#include "Value.h"

namespace Agape
{

namespace Carlo
{

namespace Expressions
{

Logical::Logical() :
  m_lhs( nullptr ),
  m_operation( unknown ),
  m_rhs( nullptr )
{
}

Logical::~Logical()
{
    delete( m_lhs );
    delete( m_rhs );
}

bool Logical::eval( Value& value, ExecutionContext& executionContext )
{
    Value lhs;
    Value rhs;
    switch( m_operation )
    {
    case logicalAnd:
        if( m_lhs->eval( lhs, executionContext ) &&
            m_rhs->eval( rhs, executionContext ) )
        {
            value = (int)( (int)lhs && (int)rhs );
            return true;
        }
        break;
    case logicalOr:
        if( m_lhs->eval( lhs, executionContext ) &&
            m_rhs->eval( rhs, executionContext ) )
        {
            value = (int)( (int)lhs || (int)rhs );
            return true;
        }
        break;
    default:
        error( ExecutionContext::errUnknownLogicalOperator, executionContext );
        break;
    }

    return false;
}

void Logical::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "LogicalExpression\n";
    strIndent( stream, indent );
    stream << "{\n";
    strIndent( stream, indent + 4 );
    stream << "Operation: " << m_operation << "\n";
    strIndent( stream, indent + 4 );
    stream << "lhs:\n";
    m_lhs->str( stream, indent + 4 );
    stream << "rhs:\n";
    m_rhs->str( stream, indent + 4 );
    strIndent( stream, indent );
    stream << "}\n";
}

} // namespace Expressions

} // namespace Carlo

} // namespace Agape
