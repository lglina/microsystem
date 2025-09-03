#ifndef AGAPE_CARLO_SYNTAX_TREE_NODE_H
#define AGAPE_CARLO_SYNTAX_TREE_NODE_H

#include "ExecutionContext.h"
#include "Lexer.h"

namespace Agape
{

class LiteStream;
class String;
class Value;

namespace Carlo
{

class SyntaxTreeNode
{
public:
    SyntaxTreeNode();
    virtual ~SyntaxTreeNode() {}

    virtual bool evalAssignable( Value*& value, ExecutionContext& executionContext ) { return false; } // Callee owns the memory.

    // Caller owns the memory. Pre-requisite: value is nothing.
    virtual bool eval( Value& value, ExecutionContext& executionContext ) = 0;

    virtual void str( LiteStream& stream, int indent ) = 0;

    void initialToken( const Lexer::Token& initialToken );
    void error( enum ExecutionContext::ErrorCodes errorCode, ExecutionContext& executionContext );

protected:
    void strIndent( LiteStream& stream, int indent );

private:
    int m_line;
    int m_column;
    int m_len;
};

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_SYNTAX_TREE_NODE_H
