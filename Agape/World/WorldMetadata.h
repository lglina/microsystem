#ifndef AGAPE_WORLD_METADATA_H
#define AGAPE_WORLD_METADATA_H

#include "Collections.h"
#include "EncryptableDecryptable.h"
#include "String.h"
#include "User.h"

namespace Agape
{

class Encryptor;
class Value;

namespace World
{

class Metadata : public EncryptableDecryptable
{
public:
    Metadata();
    virtual ~Metadata() {}

    void toValue( Value& value, bool keys = false, bool users = true ) const;
    static Metadata fromValue( const Value& value, bool keys = false, bool users = true );

    virtual bool encrypt( Encryptor& encryptor );
    virtual bool decrypt( Encryptor& encryptor );

    String m_name;
    String m_worldID;
    String m_worldAuthKey;
    String m_worldKey;
    String m_itemKey;
    String m_privateKey;

    bool m_writable;
    Vector< User > m_users;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_WORLD_METADATA_H
