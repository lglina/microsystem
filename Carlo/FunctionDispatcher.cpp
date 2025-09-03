#include "Actors/Actor.h"
#include "Loggers/Logger.h"
#include "Collections.h"
#include "FunctionDispatcher.h"
#include "InbuiltFunctions.h"
#include "String.h"
#include "Value.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace Carlo
{

void FunctionDispatcher::registerActor( Actor* actor )
{
    m_actors[ actor->actorName() ] = actor;
}

void FunctionDispatcher::deregisterActor( Actor* actor )
{
    m_actors.erase( actor->actorName() );
}

bool FunctionDispatcher::dispatch( Value& returnValue,
                                   const String& actorName,
                                   const String& functionName,
                                   Map< String, Value* > arguments,
                                   const String& caller )
{
    if( !actorName.empty() )
    {
        Map< String, Actor* >::iterator it( m_actors.find( actorName ) );
        if( it != m_actors.end() )
        {
            return it->second->perform( returnValue, functionName, arguments, caller );
        }
    }
    else
    {
        return m_inbuiltFunctions.perform( returnValue, functionName, arguments, caller );
    }

    return false;
}

bool FunctionDispatcher::getPersistableValue( Value& value,
                                              const String& actorName,
                                              const String& valueName,
                                              const String& caller )
{
    if( !actorName.empty() )
    {
        Map< String, Actor* >::iterator it( m_actors.find( actorName ) );
        if( it != m_actors.end() )
        {
            return it->second->getPersistableValue( value, valueName, caller );
        }
    }

    return false;
}

} // namespace Linda2

} // namespace Agape
