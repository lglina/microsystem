#ifndef AGAPE_CARLO_STATEMENTS_MAKES_H
#define AGAPE_CARLO_STATEMENTS_MAKES_H

#include "Statement.h"

namespace Agape
{

class LiteStream;

namespace Carlo
{

class Block;
class ExecutionContext;
class Expression;
class Parser;

namespace Statements
{

class Makes : public Statement
{
    friend Parser;

public:
    Makes();
    virtual ~Makes();

    virtual bool eval( Value& value, ExecutionContext& executionContext );

    virtual void str( LiteStream& stream, int indent );

private:
    Expression* m_lhs;
    Expression* m_rhs;
};

} // namespace Statements

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_STATEMENTS_MAKES_H
