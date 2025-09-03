#include "Platforms/Platform.h"
#include "Collections.h"
#include "FunctionDispatcher.h"
#include "PlatformActor.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"

namespace Agape
{

namespace Linda2
{

namespace Actors
{

namespace NativeActors
{

Platform::Platform( Carlo::FunctionDispatcher& functionDispatcher,
                    Agape::Platform& platform ) :
  Linda2::Actors::Native( "Computer" ),
  m_functionDispatcher( functionDispatcher ),
  m_platform( platform )
{
    m_functionDispatcher.registerActor( this );
}

Platform::~Platform()
{
    m_functionDispatcher.deregisterActor( this );
}

bool Platform::perform( Value& returnValue,
                        const String& name,
                        Map< String, Value* > arguments,
                        const String& caller )
{
    if( name == _userRead )
    {
        if( arguments.find( _bit ) != arguments.end() )
        {
            int bit = *( arguments[_bit] );
            returnValue = m_platform.userRead( bit );
        }

        return true;
    }
    else if( name == _userWrite )
    {
        if( ( arguments.find( _bit ) != arguments.end() ) &&
            ( arguments.find( _value ) != arguments.end() ) )
        {
            int bit = *( arguments[_bit] );
            int value = *( arguments[_value] );
            m_platform.userWrite( bit, value );
        }

        return true;
    }
    else if( name == "test" )
    {
        m_platform.testBus();
    }

    return false;
}

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape
