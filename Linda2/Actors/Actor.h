#ifndef AGAPE_LINDA2_ACTOR_H
#define AGAPE_LINDA2_ACTOR_H

#include "Utils/LiteStream.h"
#include "Collections.h"
#include "ExecutionContext.h"
#include "SyntaxTreeNode.h"
#include "Value.h"

using namespace Agape::Carlo;

namespace Agape
{

class String;

namespace Linda2
{

class Tuple;

class Actor : public SyntaxTreeNode
{
public:
    virtual ~Actor() {}

    virtual bool eval( Value& value, ExecutionContext& executionContext ) { return false; };

    // Handles a tuple.
    // FIXME: Should be passed by const reference, but then we need to make
    // Carlo const-correct, i.e. ExecutionContext will need separate vectors
    // of tuple pointers (const and non-const) and IdentifierExpression and
    // other nodes will need to look for and handle both const and non-const
    // tuples in the execution context as appropriate.
    virtual bool accept( Tuple& tuple ) = 0;

    // Performs some computation and returns a value.
    virtual bool perform( Value& returnValue,
                          const String& name,
                          Map< String, Value* > arguments,
                          const String& caller ) = 0;
    
    // Returns a value and attaches a ValueLoader so the caller can modify
    // and save the value.
    virtual bool getPersistableValue( Value& value,
                                      const String& name,
                                      const String& caller ) { return false; };

    virtual String actorName() const = 0;
    virtual void rename( const String& name ) {};
};

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_ACTOR_H
