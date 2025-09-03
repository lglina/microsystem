#include "Encryptors/Encryptor.h"
#include "String.h"
#include "StringConstants.h"
#include "Telegram.h"
#include "Value.h"

namespace Agape
{

namespace World
{

Telegram::Telegram() :
  m_dateTime( 0 ),
  m_unread( true )
{
}

void Telegram::toValue( Value& value ) const
{
    value[_telegramSnowflake] = m_telegramSnowflake;
    value[_senderSnowflake] = m_senderSnowflake;
    value[_recipientSnowflake] = m_recipientSnowflake;
    value[_dateTime] = m_dateTime;
    value[_unread] = m_unread ? 1 : 0;
    value[_subject] = m_subject;
}

Telegram Telegram::fromValue( const Value& value )
{
    Telegram telegram;
    telegram.m_telegramSnowflake = value[_telegramSnowflake];
    telegram.m_senderSnowflake = value[_senderSnowflake];
    telegram.m_recipientSnowflake = value[_recipientSnowflake];
    telegram.m_dateTime = value[_dateTime];
    telegram.m_unread = ( (int)value[_unread] == 1 );
    telegram.m_subject = value[_subject];

    return telegram;
}

bool Telegram::encrypt( Encryptor& encryptor )
{
    m_subject = encryptor.encrypt( m_subject );

    return true;
}

bool Telegram::decrypt( Encryptor& encryptor )
{
    m_subject = encryptor.decrypt( m_subject );

    return true;
}

bool Telegram::operator==( const Telegram& other ) const
{
    return( other.m_telegramSnowflake == m_telegramSnowflake );
}

bool Telegram::newer( const Telegram& a, const Telegram& b )
{
    return( a.m_dateTime > b.m_dateTime );
}

} // namespace World

} // namespace Agape
