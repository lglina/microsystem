#ifndef AGAPE_CARLO_STATEMENTS_WHILE_H
#define AGAPE_CARLO_STATEMENTS_WHILE_H

#include "Statement.h"
#include "String.h"

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

class While : public Statement
{
    friend Parser;

public:
    While();
    virtual ~While();

    virtual bool eval( Value& value, ExecutionContext& executionContext );

    virtual void str( LiteStream& stream, int indent );

private:
    Expression* m_condition;
    Block* m_block;
};

} // namespace Statements

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_STATEMENTS_WHILE_H
