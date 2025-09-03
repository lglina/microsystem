#ifndef AGAPE_LINDA2_ACTORS_LINDA_ACTOR_H
#define AGAPE_LINDA2_ACTORS_LINDA_ACTOR_H

#include "Actor.h"

#include "Collections.h"
#include "ExecutionContext.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace Carlo
{
class Parser;
class ProgramManager;
} // namespace Carlo

class LiteStream;

using namespace Carlo;

namespace Linda2
{

class Tuple;
class TupleHandler;
class TupleRouter;

namespace Actors
{

class Linda2Actor : public Actor
{
    friend Parser;
    friend ProgramManager;

public:
    Linda2Actor( const String& name, TupleRouter& tupleRouter );
    virtual ~Linda2Actor();

    void doRegister();

    virtual bool accept( Tuple& tuple );
    
    // Pre-requisite: returnValue is nothing.
    virtual bool perform( Value& returnValue,
                          const String& name,
                          Map< String, Value* > arguments,
                          const String& caller ) { return false; };
    
    virtual bool getPersistableValue( Value& value,
                                      const String& name,
                                      const String& caller ) { return false; };

    virtual String actorName() const;
    virtual void rename( const String& name );

    virtual void str( LiteStream& stream, int indent );

    const Vector< ExecutionContext::RuntimeError > runtimeErrors() const;

    bool evalOne( Value& value, ExecutionContext& executionContext );

private:
    String m_name;
    TupleRouter& m_tupleRouter;

    Vector< TupleHandler* > m_tupleHandlers;
};

} // namespace Actors

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_ACTORS_LINDA_ACTOR_H
