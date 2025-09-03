#ifndef AGAPE_CARLO_STATEMENTS_STOP_H
#define AGAPE_CARLO_STATEMENTS_STOP_H

#include "Statement.h"
#include "String.h"

namespace Agape
{

class LiteStream;

namespace Carlo
{

class ExecutionContext;
class Parser;

namespace Statements
{

class Stop : public Statement
{
    friend Parser;

public:
    virtual bool eval( Value& value, ExecutionContext& executionContext );

    virtual void str( LiteStream& stream, int indent );
};

} // namespace Statements

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_STATEMENTS_STOP_H
