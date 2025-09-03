#include "PresenceLoaders/PresenceRequest.h"
#include "Utils/Cartesian.h"
#include "Utils/Snowflake.h"
#include "ValueLoaders/SceneItemValueLoader.h"
#include "World/Compositor.h"
#include "World/SceneItem.h"
#include "World/ScenePresence.h"
#include "World/User.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "FunctionDispatcher.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "TupleRouter.h"
#include "Tuple.h"
#include "WorldActor.h"
#include "Worldbook.h"

#include <string.h>

using namespace Agape::World;

namespace Agape
{

namespace Linda2
{

namespace Actors
{

namespace NativeActors
{

World::World( TupleRouter& tupleRouter,
              FunctionDispatcher& functionDispatcher,
              Compositor& compositor,
              Agape::Snowflake& snowflake,
              Coordinates& coordinates,
              Worldbook& worldbook ) :
  Native( "World" ),
  m_tupleRouter( tupleRouter ),
  m_functionDispatcher( functionDispatcher ),
  m_compositor( compositor ),
  m_snowflake( snowflake ),
  m_coordinates( coordinates ),
  m_worldbook( worldbook ),
  m_prevX( 0 ),
  m_prevY( 0 )
{
    m_tupleRouter.registerActor( this );
    m_functionDispatcher.registerActor( this );
}

World::~World()
{
    m_tupleRouter.deregisterActor( this );
    m_functionDispatcher.deregisterActor( this );
}

bool World::accept( Tuple& tuple )
{
    bool handled( false );

    String tupleType( TupleRouter::tupleType( tuple ) );
    if( TupleRouter::destinationActor( tuple ) == _World )
    {
        // FIXME: Allow things to have alias names, and translate that
        // to snowflakes here?
        String snowflake;
        if( tuple.hasValue( _id ) )
        {
            snowflake = tuple[_id];
        }
        else
        {
            snowflake = TupleRouter::sourceActor( tuple );
        }

        if( tupleType == _CreatePerson )
        {
            String name = tuple[_name];
            int glyph = tuple.hasValue( _glyph ) ? (int)tuple[_glyph] : 128;
            String colour = tuple[_colour];
            int attributes = !colour.empty() ? Terminal::attributes( colour ) : 7;
            int row = tuple.hasValue( _row ) ? (int)tuple[_row] : m_compositor.height() / 2;
            int col = tuple.hasValue( _column ) ? (int)tuple[_column] : m_compositor.width() / 2;
            m_compositor.createPerson( name, glyph, attributes, row, col );
            handled = true;
        }
        else if( tupleType == _MovePerson )
        {
            // Look for tuple[_name] and look up snowflake for name...
            if( tuple.hasValue(_direction) )
            {
                Direction direction( Direction::fromValue( tuple[_direction] ) );
                m_compositor.movePerson( snowflake, direction.m_direction );
                handled = true;
            }
            else if( tuple.hasValue( _row ) && tuple.hasValue( _column ) )
            {
                int row( tuple[_row] );
                int col( tuple[_column] );
                m_compositor.movePerson( snowflake, row, col );
                handled = true;
            }
        }
        else if( tupleType == _TeleportPerson )
        {
            Coordinates coordinates( m_coordinates );
            int row( -1 );
            int col( -1 );

            if( tuple.hasValue( _world ) )
            {
                String worldID;
                if( m_worldbook.getWorldIDByName( tuple[_world], worldID ) )
                {
                    coordinates.m_worldID = worldID;
                }
            }

            if( tuple.hasValue( _x ) ) coordinates.m_x = tuple[_x];
            if( tuple.hasValue( _y ) ) coordinates.m_y = tuple[_y];

            if( tuple.hasValue( _row ) ) row = tuple[_row];
            if( tuple.hasValue( _column ) ) col = tuple[_column];

            m_compositor.requestTeleport( coordinates, row, col );
            handled = true;
        }
        else if( tupleType == _DeletePerson )
        {
            // Look for tuple[_name] and look up snowflake for name...
            m_compositor.deletePerson( snowflake );
            handled = true;
        }
        else if( tupleType == _CreateThing )
        {
            if( tuple.hasValue( _name ) &&
                tuple.hasValue( _row ) &&
                tuple.hasValue( _column ) )
            {
                String name = tuple[_name];
                String action;
                int row = tuple[_row];
                int col = tuple[_column];

                if( tuple.hasValue( _action ) )
                {
                    action = tuple[_action];
                }

                m_compositor.createItem( name,
                                         action,
                                         row,
                                         col );
                handled = true;
            }
        }
        else if( tupleType == _MoveThing )
        {
            if( tuple.hasValue(_direction) )
            {
                Direction direction( Direction::fromValue( tuple[_direction] ) );
                m_compositor.moveItem( snowflake, direction.m_direction );
                handled = true;
            }
            else if( tuple.hasValue( _row ) && tuple.hasValue( _column ) )
            {
                int row( tuple[_row] );
                int col( tuple[_column] );
                m_compositor.moveItem( snowflake, row, col );
                handled = true;
            }
        }
        else if( tupleType == _ChangeThing )
        {
            if( tuple.hasValue( _name ) )
            {
                String name = tuple[_name];
                String action;
                bool haveAction( false );
                if( tuple.hasValue( _action ) )
                {
                    action = tuple[_action];
                    haveAction = true;
                }

                m_compositor.updateItem( snowflake, name, action, false, haveAction ); // false = not immediate
                handled = true;
            }
        }
        else if( tupleType == _TransportThing )
        {
            if( tuple.hasValue( _x ) && tuple.hasValue( _y ) )
            {
                int x( tuple[_x] );
                int y( tuple[_y] );

                // -1 = move to other scene, keep row, col.
                int row = tuple.hasValue( _row ) ? (int)tuple[_row] : -1;
                int col = tuple.hasValue( _column ) ? (int)tuple[_column] : -1;

                m_compositor.transportItem( snowflake, x, y, row, col );
                handled = true;
            }
        }
        else if( tupleType == _DeleteThing )
        {
            m_compositor.deleteItem( snowflake );
        }
        else if( tupleType == _DrawText )
        {
            if( tuple.hasValue( _text ) )
            {
                const SceneItem* item( m_compositor.findItemBySnowflake( snowflake ) );
                if( item )
                {
                    Value& textValue( tuple[_text] );
                    String textString;
                    if( textValue.type() == Value::word )
                    {
                        textString = textValue;
                    }
                    else if( textValue.type() == Value::number )
                    {
                        double numberValue = textValue;
                        if( (int)numberValue == numberValue )
                        {
                            LiteStream stream;
                            stream << (int)numberValue;
                            textString = stream.str();
                        }
                        else
                        {
                            LiteStream stream;
                            stream << numberValue;
                            textString = stream.str();
                        }
                    }
                    m_compositor.drawTextItem( *item, textString, true ); // true = clearFirst.
                    handled = true;
                }
            }
        }
    }

    return handled;
}

bool World::perform( Value& returnValue,
                     const String& name,
                     Map< String, Value* > arguments,
                     const String& caller )
{
    if( name == _nearbyThings )
    {
        const SceneItem* thisItem( m_compositor.findItemBySnowflake( caller ) );
        if( thisItem )
        {
            Rectangle thisItemRectangle( thisItem->col(),
                                         thisItem->row(),
                                         thisItem->height(),
                                         thisItem->width() );

            Set< const SceneItem* > nearbyItems( m_compositor.findNearbyItems( thisItem ) );
            Set< const SceneItem* >::const_iterator it( nearbyItems.begin() );
            for( ; it != nearbyItems.end(); ++it )
            {
                const SceneItem* otherItem( *it );

                Value* itemValue = new Value();
                (*itemValue)[_name] = otherItem->assetName();
                (*itemValue)[_id] = otherItem->snowflake();
                (*itemValue)[_row] = otherItem->row();
                (*itemValue)[_column] = otherItem->col();

                Rectangle otherItemRectangle( otherItem->col(),
                                              otherItem->row(),
                                              otherItem->height(),
                                              otherItem->width() );
                enum Rectangle::Edge edge( thisItemRectangle.edgeTouches( otherItemRectangle ) );

                switch( edge )
                {
                case Rectangle::edgeTop:
                    (*itemValue)[_edge] = _top;
                    break;
                case Rectangle::edgeBottom:
                    (*itemValue)[_edge] = _bottom;
                    break;
                case Rectangle::edgeLeft:
                    (*itemValue)[_edge] = _left;
                    break;
                case Rectangle::edgeRight:
                    (*itemValue)[_edge] = _right;
                    break;
                case Rectangle::edgeNone:
                default:
                    (*itemValue)[_edge] = _none;
                    break;
                }

                if( otherItemRectangle.intersects( thisItemRectangle ) )
                {
                    (*itemValue)[_overlap] = 1;
                }
                else
                {
                    (*itemValue)[_overlap] = 0;
                }

                returnValue.push_back( itemValue );
            }
        }

        return true;
    }
    else if( name == _nearbyPeople )
    {
        const SceneItem* thisItem( m_compositor.findItemBySnowflake( caller ) );
        if( thisItem )
        {
            Set< const ScenePresence* > nearbyUsers( m_compositor.findNearbyUsers( thisItem ) );
            Set< const ScenePresence* >::const_iterator it( nearbyUsers.begin() );
            for( ; it != nearbyUsers.end(); ++it )
            {
                Value* userValue = new Value();
                (*userValue)[_id] = (*it)->m_user.m_snowflake;
                (*userValue)[_name] = (*it)->m_user.m_name;
                returnValue.push_back( userValue );
            }
        }

        return true;
    }
    else if( name == _findThing )
    {
        if( arguments.find( _action ) != arguments.end() )
        {
            String action = *( arguments[_action] );
            const SceneItem* item( m_compositor.findItemByAction( action ) );
            if( item )
            {
                item->toValue( returnValue );
            }
        }

        return true;
    }
    else if( name == _findThings )
    {
        if( arguments.find( _name ) != arguments.end() )
        {
            String name = *( arguments[_name] );
            Set< const SceneItem* > items( m_compositor.findItems( name ) );
            Set< const SceneItem* >::const_iterator it( items.begin() );
            for( ; it != items.end(); ++it )
            {
                const SceneItem* item( *it );

                Value* itemValue = new Value();
                (*itemValue)[_id] = item->snowflake();
                (*itemValue)[_row] = item->row();
                (*itemValue)[_column] = item->col();
                (*itemValue)[_height] = item->height();
                (*itemValue)[_width] = item->width();

                returnValue.push_back( itemValue );
            }
        }

        return true;
    }
    else if( name == _itemAt )
    {
        if( ( arguments.find( _row ) != arguments.end() ) &&
            ( arguments.find( _column ) != arguments.end() ) )
        {
            int row = *( arguments[_row] );
            int col = *( arguments[_column] );

            int height( 1 );
            int width( 1 );

            if( ( arguments.find( _height ) != arguments.end() ) &&
                ( arguments.find( _width ) != arguments.end() ) )
            {
                height = *( arguments[_height] );
                width = *( arguments[_width] );
            }

            const SceneItem* item( m_compositor.itemAt( row, col, height, width ) );
            if( item )
            {
                item->toValue( returnValue );
            }
        }

        return true;
    }
    else if( name == _coordinates )
    {
        m_coordinates.toValue( returnValue );

        return true;
    }
    else if( name == _getHeight )
    {
        if( ( arguments.find( _row ) != arguments.end() ) &&
            ( arguments.find( _column ) != arguments.end() ) )
        {
            int row = *( arguments[_row] );
            int col = *( arguments[_column] );

            returnValue = m_compositor.getHeightAt( row, col );
        }

        return true;
    }
    else if( name == _setHeight )
    {
        if( ( arguments.find( _row ) != arguments.end() ) &&
            ( arguments.find( _column ) != arguments.end() ) &&
            ( arguments.find( _height ) != arguments.end() ) )
        {
            int row = *( arguments[_row] );
            int col = *( arguments[_column] );
            int newHeight = *( arguments[_height] );

            m_compositor.setHeightAt( row, col, newHeight );
        }

        return true;
    }

    return false;
}

bool World::getPersistableValue( Value& value,
                                 const String& name,
                                 const String& caller )
{
    bool success( true );

    // Kludge - use scene item attributes to create persistable values for the
    // World actor. See comments in NativeActors::Thing::getPersistableValue().
    if( !m_compositor.hasSceneItemAttribute( _World, name ) )
    {
        success = m_compositor.createSceneItemAttribute( _World, name );
    }

    if( success )
    {
        value.setValueLoader( new ValueLoaders::SceneItem( m_compositor,
                                                           _World,
                                                           name ) );
        success = value.load();
    }

    return success;
}

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape
