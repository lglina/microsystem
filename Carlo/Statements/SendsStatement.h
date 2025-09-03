#ifndef AGAPE_CARLO_STATEMENTS_SENDS_H
#define AGAPE_CARLO_STATEMENTS_SENDS_H

#include "Statement.h"

#include "Collections.h"
#include "String.h"

namespace Agape
{

namespace Linda2
{
class TupleRouter;
} // namespace Linda2

class LiteStream;
class Value;

namespace Carlo
{

class ExecutionContext;
class Expression;
class Parser;

namespace Statements
{

class Sends : public Statement
{
    friend Parser;

public:
    Sends( TupleRouter& tupleRouter );
    virtual ~Sends();

    virtual bool eval( Value& value, ExecutionContext& executionContext );

    virtual void str( LiteStream& stream, int indent );

private:
    enum Type
    {
        t_unknown,
        t_tupleLiteral,
        t_namedTuple
    };

    TupleRouter& m_tupleRouter;

    enum Type m_type;

    // For literal.
    Expression* m_destinationActor;
    Expression* m_destinationID;
    String m_tupleType;
    Map< String, Expression* > m_tupleValues;

    // For named tuple.
    String m_tupleName;
};

} // namespace Statements

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_STATEMENTS_SENDS_H
