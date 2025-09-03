#include "Utils/Snowflake.h"
#include "Collections.h"
#include "FunctionDispatcher.h"
#include "SnowflakeActor.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace Linda2
{

namespace Actors
{

namespace NativeActors
{

Snowflake::Snowflake( Carlo::FunctionDispatcher& functionDispatcher ) :
  Linda2::Actors::Native( "Snowflake" ),
  m_functionDispatcher( functionDispatcher )
{
    m_functionDispatcher.registerActor( this );
}

Snowflake::~Snowflake()
{
    m_functionDispatcher.deregisterActor( this );
}

bool Snowflake::perform( Value& returnValue,
                         const String& name,
                         Map< String, Value* > arguments,
                         const String& caller )
{
    if( name == "generate" )
    {
        returnValue = Agape::Snowflake::generate();
        return true;
    }

    return false;
}

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape
