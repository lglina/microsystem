#include "Encryptors/Encryptor.h"
#include "Utils/Snowflake.h"
#include "Collections.h"
#include "EncryptableDecryptable.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"
#include "User.h"

namespace Agape
{

namespace World
{

User::User() :
  m_snowflake( Snowflake::generate() ),
  m_glyph( 128 ),
  m_attributes( 7 )
{
}

bool User::operator==( const User& other ) const
{
    return( m_snowflake == other.m_snowflake );
}

bool User::operator!=( const User& other ) const
{
    return( !( operator==( other ) ) );
}

void User::toValue( Value& value ) const
{
    value[_snowflake] = m_snowflake;
    value[_name] = m_name;
    value[_glyph] = m_glyph;
    value[_attributes] = m_attributes;
}

User User::fromValue( const Value& value )
{
    User user;
    user.m_snowflake = value[_snowflake];
    user.m_name = value[_name];
    user.m_glyph = value[_glyph];
    user.m_attributes = value[_attributes];

    return user;
}

bool User::encrypt( Encryptor& encryptor )
{
    m_name = encryptor.encrypt( m_name );

    return true;
}

bool User::decrypt( Encryptor& encryptor )
{
    m_name = encryptor.decrypt( m_name );

    return true;
}

} // namespace World

} // namespace Agape
