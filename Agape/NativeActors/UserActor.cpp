#include "World/Compositor.h"
#include "World/User.h"
#include "FunctionDispatcher.h"
#include "String.h"
#include "StringConstants.h"
#include "TupleRouter.h"
#include "UserActor.h"

using namespace Agape::World;

namespace Agape
{

namespace Linda2
{

namespace Actors
{

namespace NativeActors
{

User::User( TupleRouter& tupleRouter,
            FunctionDispatcher& functionDispatcher,
            Compositor& compositor,
            World::User& worldUser ) :
  Native( _my ),
  m_tupleRouter( tupleRouter ),
  m_functionDispatcher( functionDispatcher ),
  m_compositor( compositor ),
  m_worldUser( worldUser )
{
    //m_tupleRouter.registerActor( this );
    m_functionDispatcher.registerActor( this );
}

User::~User()
{
    //m_tupleRouter.deregisterActor( this );
    m_functionDispatcher.deregisterActor( this );
}

bool User::accept( Tuple& tuple )
{
    bool handled( false );

    return handled;
}

bool User::perform( Value& returnValue,
                    const String& name,
                    Map< String, Value* > arguments,
                    const String& caller )
{
    if( name == _id )
    {
        returnValue = m_worldUser.m_snowflake;
        return true;
    }
    else if( name == _computerid )
    {
        returnValue = m_tupleRouter.myID();
        return true;
    }
    else if( name == _row )
    {
        returnValue = m_compositor.positionRow();
        return true;
    }
    else if( name == _column )
    {
        returnValue = m_compositor.positionCol();
        return true;
    }

    return false;
}

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape
