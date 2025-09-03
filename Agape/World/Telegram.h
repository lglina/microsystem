#ifndef AGAPE_WORLD_TELEGRAM_H
#define AGAPE_WORLD_TELEGRAM_H

#include "EncryptableDecryptable.h"
#include "String.h"

namespace Agape
{

class Encryptor;
class Value;

namespace World
{

class Telegram : public EncryptableDecryptable
{
public:
    Telegram();

    void toValue( Value& value ) const;
    static Telegram fromValue( const Value& value );

    virtual bool encrypt( Encryptor& encryptor );
    virtual bool decrypt( Encryptor& encryptor );

    bool operator==( const Telegram& other ) const;

    static bool newer( const Telegram& a, const Telegram& b );

    String m_telegramSnowflake;
    String m_senderSnowflake;
    String m_recipientSnowflake;
    int m_dateTime;
    bool m_unread;
    String m_subject;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_WORLD_TELEGRAM_H
