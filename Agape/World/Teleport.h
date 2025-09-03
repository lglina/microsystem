#ifndef AGAPE_WORLD_TELEPORT_H
#define AGAPE_WORLD_TELEPORT_H

#include "World/WorldCoordinates.h"
#include "EncryptableDecryptable.h"
#include "String.h"

namespace Agape
{

class Encryptor;
class Value;

namespace World
{

class Teleport : public EncryptableDecryptable
{
public:
    Teleport();

    void toValue( Value& value ) const;
    static Teleport fromValue( const Value& value );

    virtual bool encrypt( Encryptor& encryptor );
    virtual bool decrypt( Encryptor& encryptor );

    bool operator==( const Teleport& other ) const;

    static bool newer( const Teleport& a, const Teleport& b );

    String m_snowflake;
    String m_name;
    int m_row;
    int m_col;
    Coordinates m_coordinates;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_WORLD_TELEPORT_H
