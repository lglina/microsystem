#ifndef AGAPE_CARLO_EXPRESSIONS_FUNCTION_H
#define AGAPE_CARLO_EXPRESSIONS_FUNCTION_H

#include "Collections.h"
#include "Expression.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

class LiteStream;

namespace Carlo
{

class ExecutionContext;
class FunctionDispatcher;
class Parser;

namespace Expressions
{

class Function : public Expression
{
    friend Parser;

public:
    Function( FunctionDispatcher& functionDispatcher );
    ~Function();

    virtual bool eval( Value& value, ExecutionContext& executionContext );

    virtual void str( LiteStream& stream, int indent );

private:
    FunctionDispatcher& m_functionDispatcher;

    String m_name;
    Map< String, Expression* > m_argumentExpressions;
};

} // namespace Expressions

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_EXPRESSIONS_FUNCTION_H
