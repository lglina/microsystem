#include "Encryptors/Encryptor.h"
#include "World/Direction.h"
#include "World/WorldCoordinates.h"
#include "PresenceRequest.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"

using namespace Agape::World;
using Agape::World::Direction;

namespace Agape
{

PresenceRequest::PresenceRequest() :
  m_direction( Direction::none ),
  m_keyboard( false )
{
}

PresenceRequest::PresenceRequest( enum PresenceOperation presenceOperation,
                                  const ScenePresence& scenePresence,
                                  const Coordinates& coordinates,
                                  Direction direction,
                                  bool push,
                                  bool keyboard ) :
  m_presenceOperation( presenceOperation ),
  m_scenePresence( scenePresence ),
  m_coordinates( coordinates ),
  m_direction( direction ),
  m_push( push ),
  m_keyboard( keyboard )
{
}

void PresenceRequest::toTuple( Linda2::Tuple& tuple ) const
{
    tuple[_presenceOperation] = presenceOperationToString( m_presenceOperation );
    m_scenePresence.toValue( tuple[_scenePresence] );
    m_coordinates.toValue( tuple[_coordinates] );
    m_direction.toValue( tuple[_direction] );
    if( !m_originatorID.empty() ) { tuple[_originatorID] = m_originatorID; }
    tuple[_push] = m_push ? 1 : 0;
    tuple[_keyboard] = m_keyboard ? 1 : 0;
}

PresenceRequest PresenceRequest::fromTuple( const Linda2::Tuple& tuple )
{
    enum PresenceOperation presenceOperation( presenceOperationFromString( tuple[_presenceOperation] ) );
    ScenePresence scenePresence( ScenePresence::fromValue( tuple[_scenePresence] ) );
    Coordinates coordinates( Coordinates::fromValue( tuple[_coordinates] ) );
    Direction direction( Direction::fromValue( tuple[_direction] ) );
    PresenceRequest presenceRequest( presenceOperation, scenePresence, coordinates, direction );
    if( tuple.hasValue( _originatorID ) ) { presenceRequest.m_originatorID = tuple[_originatorID]; }
    presenceRequest.m_push = ( (int)tuple[_push] == 1 );
    presenceRequest.m_keyboard = ( (int)tuple[_keyboard] == 1 );
    return presenceRequest;
}

bool PresenceRequest::encrypt( Encryptor& encryptor )
{
    return m_scenePresence.encrypt( encryptor );
}

bool PresenceRequest::decrypt( Encryptor& encryptor )
{
    return m_scenePresence.decrypt( encryptor );
}

char PresenceRequest::presenceOperationToChar( enum PresenceOperation operation )
{
    switch( operation )
    {
    case arrive:
        return 0;
    case depart:
        return 1;
    case move:
        return 2;
    default:
        return -1;
    }
}

enum PresenceRequest::PresenceOperation PresenceRequest::presenceOperationFromChar( char operation )
{
    switch( operation )
    {
    case 0:
        return arrive;
    case 1:
        return depart;
    case 2:
        return move;
    default:
        return unknown;
    }
}

String PresenceRequest::presenceOperationToString( enum PresenceOperation operation )
{
    switch( operation )
    {
    case arrive:
        return "arrive";
    case depart:
        return "depart";
    case move:
        return "move";
    default:
        return "unknown";
    }
}

enum PresenceRequest::PresenceOperation PresenceRequest::presenceOperationFromString( const String& operation )
{
    if( operation == "arrive" )
    {
        return arrive;
    }
    else if( operation == "depart" )
    {
        return depart;
    }
    else if( operation == "move" )
    {
        return move;
    }
    else
    {
        return unknown;
    }
}

} // namespace Agape
