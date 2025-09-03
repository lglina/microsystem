#ifndef AGAPE_LINDA2_TUPLE_HANDLER_H
#define AGAPE_LINDA2_TUPLE_HANDLER_H

#include "Utils/LiteStream.h"
#include "Collections.h"
#include "ExecutionContext.h"
#include "String.h"
#include "SyntaxTreeNode.h"

namespace Agape
{

namespace Carlo
{
class Block;
class Expression;
class Parser;
class ProgramManager;
} // namespace Carlo

using namespace Carlo;

namespace Linda2
{

namespace Actors
{
class Linda2Actor;
} // namespace Actors

class Tuple;

class TupleHandler : public SyntaxTreeNode
{
    friend Actors::Linda2Actor;
    friend Parser;
    friend ProgramManager;

public:
    TupleHandler();
    ~TupleHandler();

    virtual bool eval( Value& value, ExecutionContext& executionContext ) { return false; };
    virtual void str( LiteStream& stream, int indent );

    bool accept( Tuple& tuple,
                 ExecutionContext& executionContext );
    
    bool evalOne( Value& value, ExecutionContext& executionContext );

private:
    String m_sourceActor;
    String m_tupleType;
    String m_tupleAlias;
    Expression* m_condition;
    Block* m_block;

    Vector< ExecutionContext::RuntimeError > m_runtimeErrors;

    int m_users;
};

} // namespace Carlo

} // namespace Linda2

#endif // AGAPE_LINDA2_TUPLE_HANDLER_H
