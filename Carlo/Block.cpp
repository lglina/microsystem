#include "Statements/Statement.h"
#include "Utils/LiteStream.h"
#include "Block.h"
#include "ExecutionContext.h"
#include "Value.h"

namespace Agape
{

namespace Carlo
{

Block::~Block()
{
    Vector< Statement* >::const_iterator it( m_statements.begin() );
    for( ; it != m_statements.end(); ++it )
    {
        delete( *it );
    }
}

bool Block::eval( Value& value, ExecutionContext& executionContext )
{
    Vector< Statement* >::const_iterator it( m_statements.begin() );
    bool success( true );
    for( ; success && ( ( it != m_statements.end() ) && !executionContext.m_stop ); ++it )
    {
        success = ( *it )->eval( value, executionContext );
    }

    return success;
}

void Block::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "Block\n";
    strIndent( stream, indent );
    stream << "{\n";
    Vector< Statement* >::const_iterator it( m_statements.begin() );
    for( ; it != m_statements.end(); ++it )
    {
        ( *it )->str( stream, indent + 4 );
    }
    strIndent( stream, indent );
    stream << "}\n";
}

} // namespace Carlo

} // namespace Agape
