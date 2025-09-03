#ifndef AGAPE_CARLO_STATEMENTS_EACH_H
#define AGAPE_CARLO_STATEMENTS_EACH_H

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

class Each : public Statement
{
    friend Parser;

public:
    Each();
    virtual ~Each();

    virtual bool eval( Value& value, ExecutionContext& executionContext );

    virtual void str( LiteStream& stream, int indent );

private:
    String m_elementName;
    Expression* m_collection;
    Block* m_block;
};

} // namespace Statements

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_STATEMENTS_EACH_H
