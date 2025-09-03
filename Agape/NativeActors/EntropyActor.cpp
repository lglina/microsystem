#include "EntropySources/EntropySource.h"
#include "Collections.h"
#include "EntropyActor.h"
#include "FunctionDispatcher.h"
#include "String.h"
#include "Value.h"

#include <stdlib.h>

namespace Agape
{

namespace Linda2
{

namespace Actors
{

namespace NativeActors
{

Entropy::Entropy( Carlo::FunctionDispatcher& functionDispatcher,
                  EntropySource& entropySource ) :
  Linda2::Actors::Native( "Random" ),
  m_functionDispatcher( functionDispatcher ),
  m_entropySource( entropySource )
{
    m_functionDispatcher.registerActor( this );
}

Entropy::~Entropy()
{
    m_functionDispatcher.deregisterActor( this );
}

bool Entropy::perform( Value& returnValue,
                       const String& name,
                       Map< String, Value* > arguments,
                       const String& caller )
{
    if( name == "number" )
    {
        int number;
        m_entropySource.generate( (char*)&number, sizeof( int ) );
        number = ::abs( number );
        if( arguments.find( "limit" ) != arguments.end() )
        {
            int limit = *( arguments["limit"] );
            number %= limit;
        }
        returnValue = number;
        return true;
    }

    return false;
}

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape
