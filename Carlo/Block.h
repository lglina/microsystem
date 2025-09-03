#ifndef AGAPE_CARLO_BLOCK_H
#define AGAPE_CARLO_BLOCK_H

#include "Utils/LiteStream.h"
#include "Collections.h"
#include "SyntaxTreeNode.h"

namespace Agape
{

namespace Carlo
{

class Parser;
class Statement;

class Block : public SyntaxTreeNode
{
    friend Parser;

public:
    virtual ~Block();

    virtual bool eval( Value& value, ExecutionContext& executionContext );

    virtual void str( LiteStream& stream, int indent );

private:
    Vector< Statement* > m_statements;
};

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_BLOCK_H
