#ifndef AGAPE_WORLD_USER_H
#define AGAPE_WORLD_USER_H

#include "Collections.h"
#include "EncryptableDecryptable.h"
#include "String.h"

namespace Agape
{

class Encryptor;
class Value;

namespace World
{

class User : public EncryptableDecryptable
{
public:
    User();
    virtual ~User() {}

    bool operator==( const User& other ) const;
    bool operator!=( const User& other ) const;

    void toValue( Value& value ) const;
    static User fromValue( const Value& value );

    virtual bool encrypt( Encryptor& encryptor );
    virtual bool decrypt( Encryptor& encryptor );

    String m_snowflake;
    String m_name;
    int m_glyph;
    int m_attributes;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_WORLD_USER_H
