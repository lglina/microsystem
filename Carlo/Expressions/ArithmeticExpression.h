#ifndef AGAPE_CARLO_EXPRESSIONS_ARITHMETIC_H
#define AGAPE_CARLO_EXPRESSIONS_ARITHMETIC_H

#include "Expression.h"
#include "Value.h"

namespace Agape
{

class LiteStream;

namespace Carlo
{

class ExecutionContext;
class Parser;

namespace Expressions
{

class Arithmetic : public Expression
{
    friend Parser;

public:
    Arithmetic();
    virtual ~Arithmetic();

    virtual bool eval( Value& value, ExecutionContext& executionContext );

    virtual void str( LiteStream& stream, int indent );

private:
    enum Operation
    {
        unknown,
        addition,
        subtraction,
        multiplication,
        division,
        modulo
    };

    Expression* m_lhs;
    enum Operation m_operation;
    Expression* m_rhs;
};

} // namespace Expressions

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_EXPRESSIONS_ARITHMETIC_H
