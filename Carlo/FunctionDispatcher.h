#ifndef AGAPE_CARLO_FUNCTION_DISPATCHER_H
#define AGAPE_CARLO_FUNCTION_DISPATCHER_H

#include "Value.h"

#include "Collections.h"
#include "InbuiltFunctions.h"
#include "String.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace Linda2
{
class Actor;
} // namespace Linda2

class Value;

namespace Carlo
{

class FunctionDispatcher
{
public:
    virtual ~FunctionDispatcher() {};

    void registerActor( Actor* actor );
    void deregisterActor( Actor* actor );

    bool dispatch( Value& returnValue,
                   const String& actorName,
                   const String& functionName,
                   Map< String, Value* > arguments,
                   const String& caller );
    
    virtual bool getPersistableValue( Value& value,
                                      const String& actorName,
                                      const String& valueName,
                                      const String& caller );

private:
    Map< String, Actor* > m_actors;

    InbuiltFunctions m_inbuiltFunctions;
};

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_FUNCTION_DISPATCHER_H
