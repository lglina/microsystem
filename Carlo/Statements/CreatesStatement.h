#ifndef AGAPE_CARLO_STATEMENTS_CREATES_H
#define AGAPE_CARLO_STATEMENTS_CREATES_H

#include "Collections.h"
#include "Statement.h"
#include "String.h"

namespace Agape
{

class LiteStream;

namespace Carlo
{

class ExecutionContext;
class Expression;
class Parser;

namespace Statements
{

class Creates : public Statement
{
    friend Parser;

public:
    Creates();
    virtual ~Creates();

    virtual bool eval( Value& value, ExecutionContext& executionContext );

    virtual void str( LiteStream& stream, int indent );

private:
    enum Type
    {
        t_unknown,
        t_tuple,
        t_value
    };

    enum Type m_type;
    
    // For tuple.
    String m_tupleName;
    String m_tupleType;
    Map< String, Expression* > m_tupleValues;

    // For value.
    String m_valueName;
    Expression* m_valueExpression;
};

} // namespace Statements

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_STATEMENTS_CREATES_H
