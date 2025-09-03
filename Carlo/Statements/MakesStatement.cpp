#include "Expressions/Expression.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "ExecutionContext.h"
#include "MakesStatement.h"
#include "Value.h"

namespace Agape
{

namespace Carlo
{

namespace Statements
{

Makes::Makes() :
  m_lhs( nullptr ),
  m_rhs( nullptr )
{
}

Makes::~Makes()
{
    delete( m_lhs );
    delete( m_rhs );
}

bool Makes::eval( Value& value, ExecutionContext& executionContext )
{
    // FIXME: Check if assignable NULL and abort assignment.
#ifdef LOG_CARLO_INTERP
    LOG_DEBUG( "Eval: MakesStatement" );
#endif
    Value* lhs;
    Value rhs;
    
    // Important to evaluate right to left, as otherwise if lhs is a
    // persistable value, any use in the rhs expression will invalidate
    // a previously stored lhs pointer.
    if( m_rhs->eval( rhs, executionContext ) )
    {
        if( m_lhs->evalAssignable( lhs, executionContext ) )
        {
#ifdef LOG_CARLO_INTERP
            LiteStream stream;
            stream << "lhs: " << lhs->dump() << " rhs: " << rhs.dump();
            LOG_DEBUG( stream.str() );
#endif
            *lhs = rhs;
            value = rhs; // Return result of assignment.

            if( lhs->save() ) // If this is a persistable value, save it now.
            {
                return true;
            }
            else
            {
                error( ExecutionContext::errUnableToSaveValue, executionContext );
            }
        }
    }

    return false;
}

void Makes::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "MakesStatement\n";
    strIndent( stream, indent );
    stream << "{\n";
    strIndent( stream, indent + 4 );
    stream << "lhs:\n";
    m_lhs->str( stream, indent + 8 );
    strIndent( stream, indent + 4 );
    stream << "rhs:\n";
    m_rhs->str( stream, indent + 8 );
    strIndent( stream, indent );
    stream << "}\n";
}

} // namespace Statements

} // namespace Carlo

} // namespace Agape
