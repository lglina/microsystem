#include "Encryptors/Encryptor.h"
#include "World/WorldCoordinates.h"
#include "String.h"
#include "StringConstants.h"
#include "Teleport.h"
#include "Value.h"

namespace Agape
{

namespace World
{

Teleport::Teleport() :
  m_row( 0 ),
  m_col( 0 )
{
}

void Teleport::toValue( Value& value ) const
{
    value[_snowflake] = m_snowflake;
    value[_name] = m_name;
    value[_row] = m_row;
    value[_column] = m_col;
    m_coordinates.toValue( value[_coordinates] );
}

Teleport Teleport::fromValue( const Value& value )
{
    Teleport teleport;

    teleport.m_snowflake = value[_snowflake];
    teleport.m_name = value[_name];
    teleport.m_row = value[_row];
    teleport.m_col = value[_column];
    teleport.m_coordinates = Coordinates::fromValue( value[_coordinates] );

    return teleport;
}

bool Teleport::encrypt( Encryptor& encryptor )
{
    m_name = encryptor.encrypt( m_name );

    return true;
}

bool Teleport::decrypt( Encryptor& encryptor )
{
    m_name = encryptor.decrypt( m_name );

    return true;
}

bool Teleport::operator==( const Teleport& other ) const
{
    return( other.m_snowflake == m_snowflake );
}

bool Teleport::newer( const Teleport& a, const Teleport& b )
{
    return( a.m_snowflake > b.m_snowflake );
}

} // namespace World

} // namespace Agape
