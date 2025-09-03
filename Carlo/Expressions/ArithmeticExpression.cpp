#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "ArithmeticExpression.h"
#include "Expression.h"
#include "Value.h"

#include <cmath>

namespace Agape
{

namespace Carlo
{

namespace Expressions
{

Arithmetic::Arithmetic() :
  m_lhs( nullptr ),
  m_operation( unknown ),
  m_rhs( nullptr )
{
}

Arithmetic::~Arithmetic()
{
    delete( m_lhs );
    delete( m_rhs );
}

bool Arithmetic::eval( Value& value, ExecutionContext& executionContext )
{
    // FIXME: Check types of lhs and rhs are equal.
    // FIXME: Rather than casting to int here, should we implement
    // operator functions in Value itself?
    // FIXME: eval() could return null!

#ifdef LOG_CARLO_INTERP
    LOG_DEBUG( "Eval: ArithmeticExpression" );
#endif

    Value lhsValue;
    Value rhsValue;
    if( !( m_lhs->eval( lhsValue, executionContext ) &&
           m_rhs->eval( rhsValue, executionContext ) ) )
    {
        return false;
    }

    double lhs( (double)lhsValue );
    double rhs( (double)rhsValue );

#ifdef LOG_CARLO_INTERP
    LiteStream stream;
    stream << "lhs: " << lhs << " rhs: " << rhs;
    LOG_DEBUG( stream.str() );
#endif

    int lhsInt( 0 );
    int rhsInt( 0 );
    switch( m_operation )
    {
    case addition:
        if( ( lhsValue.type() == Value::word ) ||
            ( rhsValue.type() == Value::word ) )
        {
            value = lhsValue.toString() + rhsValue.toString();
        }
        else
        {
            value = ( lhs + rhs );
        }
        return true;
        break;
    case subtraction:
        value = ( lhs - rhs );
        return true;
        break;
    case multiplication:
        value = ( lhs * rhs );
        return true;
        break;
    case division:
        value = ( lhs / rhs );
        return true;
        break;
    case modulo:
        lhsInt = std::round( lhs );
        rhsInt = std::round( rhs );
        value = ( lhsInt % rhsInt );
        return true;
        break;
    default:
        error( ExecutionContext::errUnknownArithmeticOperator, executionContext );
        break;
    }

    return false;
}

void Arithmetic::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "ArithmeticExpression\n";
    strIndent( stream, indent );
    stream << "{\n";
    strIndent( stream, indent + 4 );
    stream << "Operation: " << m_operation << "\n";
    strIndent( stream, indent + 4 );
    stream << "lhs:\n";
    m_lhs->str( stream, indent + 8 );
    strIndent( stream, indent + 4 );
    stream << "rhs:\n";
    m_rhs->str( stream, indent + 8 );
    strIndent( stream, indent );
    stream << "}\n";
}

} // namespace Expressions

} // namespace Carlo

} // namespace Agape
