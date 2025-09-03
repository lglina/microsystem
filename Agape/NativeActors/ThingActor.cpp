#include "SceneLoaders/SceneRequest.h"
#include "ValueLoaders/SceneItemValueLoader.h"
#include "World/Compositor.h"
#include "World/SceneItem.h"
#include "FunctionDispatcher.h"
#include "String.h"
#include "StringConstants.h"
#include "ThingActor.h"
#include "TupleRouter.h"

namespace Agape
{

namespace Linda2
{

namespace Actors
{

namespace NativeActors
{

Thing::Thing( TupleRouter& tupleRouter,
              FunctionDispatcher& functionDispatcher,
              Compositor& compositor ) :
  Native( _this ),
  m_tupleRouter( tupleRouter ),
  m_functionDispatcher( functionDispatcher ),
  m_compositor( compositor )
{
    //m_tupleRouter.registerActor( this );
    m_functionDispatcher.registerActor( this );
}

Thing::~Thing()
{
    //m_tupleRouter.deregisterActor( this );
    m_functionDispatcher.deregisterActor( this );
}

bool Thing::accept( Tuple& tuple )
{
    return false;
}

bool Thing::perform( Value& returnValue,
                    const String& name,
                    Map< String, Value* > arguments,
                    const String& caller )
{
    if( name == _id )
    {
        returnValue = caller;
        return true;
    }
    else if( name == _name )
    {
        const SceneItem* sceneItem( m_compositor.findItemBySnowflake( caller ) );
        if( sceneItem )
        {
            returnValue = sceneItem->assetName();
            return true;
        }
    }
    else if( name == _row )
    {
        const SceneItem* sceneItem( m_compositor.findItemBySnowflake( caller ) );
        if( sceneItem )
        {
            returnValue = int( sceneItem->row() );
            return true;
        }
    }
    else if( name == _column )
    {
        const SceneItem* sceneItem( m_compositor.findItemBySnowflake( caller ) );
        if( sceneItem )
        {
            returnValue = int( sceneItem->col() );
            return true;
        }
    }
    else if( name == _height )
    {
        const SceneItem* sceneItem( m_compositor.findItemBySnowflake( caller ) );
        if( sceneItem )
        {
            returnValue = int( sceneItem->height() );
            return true;
        }
    }
    else if( name == _width )
    {
        const SceneItem* sceneItem( m_compositor.findItemBySnowflake( caller ) );
        if( sceneItem )
        {
            returnValue = int( sceneItem->width() );
            return true;
        }
    }

    return false;
}

bool Thing::getPersistableValue( Value& value,
                                 const String& name,
                                 const String& caller )
{
    bool success( true );

    // When an actor requests a persistable value via "this.x", create that
    // value as a scene item attribute. This is obvious for the first actor
    // in any program loaded via a scene item, as that actor will have the same
    // name as the associated scene item. For the first actor in a world or
    // scene program, kludge that by allowing a scene item attribute to be
    // created with the snowflake field set to "World" or "scene_x_y". For
    // otherly-named actors, refuse to return a value. Note that you could
    // override this logic here, but that would be a bad idea as the Stratus
    // server doesn't encrypt the "snowflake" field (so we can identify
    // orphaned attributes in the database), and so you'd be exposing the names
    // of your actors!
    
    // FIXME: Generalise this mechanism to some sort of global key-value store
    // and have it operate via some mechanism other than SceneLoader?

    if( caller != "Immediate" &&
        ( m_compositor.findItemBySnowflake( caller ) ||
          ( caller == _World ) ||
          ( caller.rfind( _scene_, 0 ) == 0 ) ) )
    {
        if( !m_compositor.hasSceneItemAttribute( caller, name ) )
        {
            success = m_compositor.createSceneItemAttribute( caller, name );
        }

        if( success )
        {
            value.setValueLoader( new ValueLoaders::SceneItem( m_compositor,
                                                               caller,
                                                               name ) );
            success = value.load();
        }
    }

    return success;
}

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape
