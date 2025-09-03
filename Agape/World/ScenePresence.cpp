#include "Encryptors/Encryptor.h"
#include "ScenePresence.h"
#include "String.h"
#include "StringConstants.h"
#include "User.h"
#include "Value.h"
#include "WorldCoordinates.h"

namespace Agape
{

namespace World
{

ScenePresence::ScenePresence() :
  m_row( 0 ),
  m_col( 0 ),
  m_present( true ),
  m_lastSeen( 0 )
{
}

void ScenePresence::toValue( Value& value ) const
{
    value[_row] = m_row;
    value[_column] = m_col;
    value[_present] = m_present ? 1 : 0;
    value[_lastSeen] = (int)m_lastSeen; // FIXME: Should be a long long in the tuple!
    m_user.toValue( value[_user] );
    m_coordinates.toValue( value[_coordinates] );
}

ScenePresence ScenePresence::fromValue( const Value& value )
{
    ScenePresence scenePresence;
    scenePresence.m_row = value[_row];
    scenePresence.m_col = value[_column];
    scenePresence.m_present = (int)value[_present] == 1 ? true : false;
    scenePresence.m_lastSeen = value[_lastSeen];
    scenePresence.m_user = User::fromValue( value[_user] );
    scenePresence.m_coordinates = Coordinates::fromValue( value[_coordinates] );
    return scenePresence;
}

bool ScenePresence::encrypt( Encryptor& encryptor )
{
    return m_user.encrypt( encryptor );
}

bool ScenePresence::decrypt( Encryptor& encryptor )
{
    return m_user.decrypt( encryptor );
}

bool ScenePresence::newer( const ScenePresence& a, const ScenePresence& b )
{
    return( a.m_lastSeen > b.m_lastSeen );
}

} // namespace World

} // namespace Agape
