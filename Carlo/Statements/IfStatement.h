#ifndef AGAPE_CARLO_STATEMENTS_IF_H
#define AGAPE_CARLO_STATEMENTS_IF_H

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

class If : public Statement
{
    friend Parser;

public:
    If();
    virtual ~If();

    virtual bool eval( Value& value, ExecutionContext& executionContext );

    virtual void str( LiteStream& stream, int indent );

private:
    Expression* m_condition;
    Block* m_block;
    Block* m_elseBlock;
};

} // namespace Statements

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_STATEMENTS_IF_H
