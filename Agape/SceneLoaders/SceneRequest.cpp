#include "Utils/Snowflake.h"
#include "World/Direction.h"
#include "World/SceneItem.h"
#include "World/WorldCoordinates.h"
#include "SceneRequest.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"

#include <string.h>

using namespace Agape::World;

namespace Agape
{

SceneRequest::SceneRequest()
{
}

SceneRequest::SceneRequest( enum SceneOperation sceneOperation,
                            const SceneItem& sceneItem,
                            const Coordinates& coordinates,
                            Direction direction,
                            bool keyboard ) :
  m_sceneOperation( sceneOperation ),
  m_sceneItem( sceneItem ),
  m_coordinates( coordinates ),
  m_direction( direction ),
  m_keyboard( keyboard )
{
}

SceneRequest::SceneRequest( const SceneItem& sceneItem,
                            const Coordinates& coordinates,
                            const Coordinates& newCoordinates ) :
  m_sceneOperation( transport ),
  m_sceneItem( sceneItem ),
  m_coordinates( coordinates ),
  m_newCoordinates( newCoordinates ),
  m_direction( Direction::none ),
  m_keyboard( false )
{
}

void SceneRequest::toTuple( Linda2::Tuple& tuple ) const
{
    tuple[_sceneOperation] = sceneOperationToString( m_sceneOperation );
    m_sceneItem.toValue( tuple[_sceneItem] );
    m_coordinates.toValue( tuple[_coordinates] );
    m_direction.toValue( tuple[_direction] );
    if( !m_originatorID.empty() ) { tuple[_originatorID] = m_originatorID; }
    tuple[_keyboard] = m_keyboard ? 1 : 0;
    if( m_sceneOperation == transport ) { m_newCoordinates.toValue( tuple[_newCoordinates] ); }
}

SceneRequest SceneRequest::fromTuple( const Linda2::Tuple& tuple )
{
    enum SceneOperation sceneOperation( sceneOperationFromString( tuple[_sceneOperation] ) );
    SceneItem sceneItem( SceneItem::fromValue( tuple[_sceneItem] ) );
    Coordinates coordinates( Coordinates::fromValue( tuple[_coordinates] ) );
    Direction direction( Direction::fromValue( tuple[_direction] ) );
    SceneRequest sceneRequest( sceneOperation, sceneItem, coordinates );
    if( tuple.hasValue( _originatorID ) ) { sceneRequest.m_originatorID = tuple[_originatorID]; }
    sceneRequest.m_keyboard = ( (int)tuple[_keyboard] == 1 );
    if( sceneOperation == transport ) { sceneRequest.m_newCoordinates = Coordinates::fromValue( tuple[_newCoordinates] ); }
    return sceneRequest;
}

bool SceneRequest::encrypt( Encryptor& encryptor )
{
    return( m_sceneItem.encrypt( encryptor ) );
}

bool SceneRequest::decrypt( Encryptor& encryptor )
{
    return( m_sceneItem.decrypt( encryptor ) );
}

bool SceneRequest::operator<( const SceneRequest& other ) const
{
    return( m_sceneItem < other.m_sceneItem );
}

char SceneRequest::sceneOperationToChar( enum SceneOperation operation )
{
    switch( operation )
    {
    case create:
        return 0;
    case update:
        return 1;
    case remove:
        return 2;
    case transport:
        return 3;
    case raise:
        return 4;
    case lower:
        return 5;
    default:
        return -1;
    }
}

enum SceneRequest::SceneOperation SceneRequest::sceneOperationFromChar( char operation )
{
    switch( operation )
    {
    case 0:
        return create;
    case 1:
        return update;
    case 2:
        return remove;
    case 3:
        return transport;
    case 4:
        return raise;
    case 5:
        return lower;
    default:
        return unknown;
    }
}

String SceneRequest::sceneOperationToString( enum SceneOperation operation )
{
    switch( operation )
    {
    case create:
        return "create";
    case update:
        return "update";
    case remove:
        return "remove";
    case transport:
        return "transport";
    case raise:
        return "raise";
    case lower:
        return "lower";
    default:
        return "unknown";
    }
}

enum SceneRequest::SceneOperation SceneRequest::sceneOperationFromString( const String& operation )
{
    if( operation == "create" )
    {
        return create;
    }
    else if( operation == "update" )
    {
        return update;
    }
    else if( operation == "remove" )
    {
        return remove;
    }
    else if( operation == "transport" )
    {
        return transport;
    }
    else if( operation == "raise" )
    {
        return raise;
    }
    else if( operation == "lower" )
    {
        return lower;
    }
    else
    {
        return unknown;
    }
}

} // namespace Agape
