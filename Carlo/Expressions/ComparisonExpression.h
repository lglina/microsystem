#ifndef AGAPE_CARLO_EXPRESSIONS_COMPARISON_H
#define AGAPE_CARLO_EXPRESSIONS_COMPARISON_H

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

class Comparison : public Expression
{
    friend Parser;

public:
    Comparison();
    virtual ~Comparison();

    virtual bool eval( Value& value, ExecutionContext& executionContext );

    virtual void str( LiteStream& stream, int indent );

private:
    enum Operation
    {
        unknown,
        equality,
        inequality,
        moreThan,
        lessThan
    };

    Expression* m_lhs;
    enum Operation m_operation;
    Expression* m_rhs;
};

} // namespace Expressions

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_EXPRESSIONS_COMPARISON_H
