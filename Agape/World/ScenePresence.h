#ifndef AGAPE_WORLD_SCENE_PRESENCE_H
#define AGAPE_WORLD_SCENE_PRESENCE_H

#include "EncryptableDecryptable.h"
#include "String.h"
#include "User.h"
#include "WorldCoordinates.h"

namespace Agape
{

class Encryptor;
class Value;

namespace World
{

class ScenePresence : public EncryptableDecryptable
{
public:
    ScenePresence();

    void toValue( Value& value ) const;
    static ScenePresence fromValue( const Value& value );

    virtual bool encrypt( Encryptor& encryptor );
    virtual bool decrypt( Encryptor& encryptor );

    static bool newer( const ScenePresence& a, const ScenePresence& b );

    int m_row;
    int m_col;
    User m_user;
    Coordinates m_coordinates;

    bool m_present;
    long long m_lastSeen;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_WORLD_SCENE_PRESENCE_H
