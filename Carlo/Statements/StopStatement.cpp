#include "Utils/LiteStream.h"
#include "ExecutionContext.h"
#include "StopStatement.h"
#include "Value.h"

namespace Agape
{

namespace Carlo
{

namespace Statements
{

bool Stop::eval( Value& value, ExecutionContext& executionContext )
{
    executionContext.m_stop = true;

    // Returns nothing.

    return true;
}

void Stop::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "StopStatement\n";
};

} // namespace Statements

} // namespace Carlo

} // namespace Agape
