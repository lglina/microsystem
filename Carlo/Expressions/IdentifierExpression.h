#ifndef AGAPE_CARLO_EXPRESSIONS_IDENTIFIER_H
#define AGAPE_CARLO_EXPRESSIONS_IDENTIFIER_H

#include "Collections.h"
#include "Expression.h"
#include "String.h"
#include "Value.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace Linda2
{
class Tuple;
} // namespace Linda2

class LiteStream;

namespace Carlo
{

class ExecutionContext;
class FunctionDispatcher;
class Parser;

namespace Expressions
{

class Identifier : public Expression
{
    friend Parser;

public:
    Identifier( FunctionDispatcher& functionDispatcher );

    // Return an lvalue, i.e. where ExecutionContext owns the memory and
    // the caller receives a pointer to a modifiable Value in ExecutionContext.
    // Note: Subsequent calls will invalidate any pointers returned from
    // previous calls.
    virtual bool evalAssignable( Value*& value, ExecutionContext& executionContext );

    // Return an rvalue, i.e. where the caller owns the memory and we
    // copy into the caller's Value.
    virtual bool eval( Value& value, ExecutionContext& executionContext );

    virtual void str( LiteStream& stream, int indent );

private:
    bool _evalAssignable( Value*& value, ExecutionContext& executionContext );
    Tuple* getNamedTuple( ExecutionContext& executionContext, const String& name );
    Value* getNamedValue( ExecutionContext& executionContext, const String& name );
    Value* getTupleValue( Tuple& tuple, const String& name );
    Value* getValueValue( Value& value, const String& name );

    String parseToken( const String& token, ExecutionContext& executionContext );

    FunctionDispatcher& m_functionDispatcher;

    String m_string; // Literal or path
    double m_float; // Literal
    bool m_quotedString;
};

} // namespace Expressions

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_EXPRESSIONS_IDENTIFIER_H
