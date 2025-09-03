#include "Utils/LiteStream.h"
#include "ComparisonExpression.h"
#include "ExecutionContext.h"
#include "Expression.h"
#include "Value.h"

namespace Agape
{

namespace Carlo
{

namespace Expressions
{

Comparison::Comparison() :
  m_lhs( nullptr ),
  m_operation( unknown ),
  m_rhs( nullptr )
{
}

Comparison::~Comparison()
{
    delete( m_lhs );
    delete( m_rhs );
}

bool Comparison::eval( Value& value, ExecutionContext& executionContext )
{
    Value lhs;
    Value rhs;
    if( m_lhs->eval( lhs, executionContext ) &&
        m_rhs->eval( rhs, executionContext ) )
    {
        switch( m_operation )
        {
        case equality:
            value = ( lhs == rhs ) ? 1 : 0;
            return true;
            break;
        case inequality:
            value = ( lhs != rhs ) ? 1 : 0;
            return true;
            break;
        case moreThan:
            value = ( (double)lhs > (double)rhs ) ? 1 : 0;
            return true;
            break;
        case lessThan:
            value = ( (double)lhs < (double)rhs ) ? 1 : 0;
            return true;
            break;
        default:
            break;
        }
    }

    return false;
}

void Comparison::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "ComparisonExpression\n";
    strIndent( stream, indent );
    stream << "{\n";
    strIndent( stream, indent + 4 );
    stream << "lhs:\n";
    m_lhs->str( stream, indent + 4 );
    stream << " rhs:\n";
    m_rhs->str( stream, indent + 4 );
    strIndent( stream, indent );
    stream << "}\n";
}

} // namespace Expressions

} // namespace Carlo

} // namespace Agape
