#include "Utils/LiteStream.h"
#include "ExecutionContext.h"
#include "String.h"
#include "SyntaxTreeNode.h"

namespace Agape
{

namespace Carlo
{

SyntaxTreeNode::SyntaxTreeNode() :
  m_line( 0 ),
  m_column( 0 ),
  m_len( 0 )
{
}

void SyntaxTreeNode::strIndent( LiteStream& stream, int indent )
{
    for( int i = 0; i < indent; ++i )
    {
        stream << " ";
    }
}

void SyntaxTreeNode::initialToken( const Lexer::Token& initialToken )
{
    m_line = initialToken.m_line;
    m_column = initialToken.m_column;
    m_len = initialToken.m_len;
}

void SyntaxTreeNode::error( enum ExecutionContext::ErrorCodes errorCode, ExecutionContext& executionContext )
{
    executionContext.m_runtimeErrors.push_back( ExecutionContext::RuntimeError( errorCode,
                                                                                m_line,
                                                                                m_column,
                                                                                m_len ) );
}

} // namespace Carlo

} // namespace Agape
