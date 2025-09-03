#ifndef AGAPE_PRESENCE_REQUEST_H
#define AGAPE_PRESENCE_REQUEST_H

#include "World/Direction.h"
#include "World/ScenePresence.h"
#include "World/WorldCoordinates.h"
#include "EncryptableDecryptable.h"
#include "String.h"

using namespace Agape::World;

namespace Agape
{

namespace Linda2
{
class Tuple;
} // namespace Linda2

class Encryptor;

class PresenceRequest : public EncryptableDecryptable
{
public:
    enum PresenceOperation
    {
        arrive,
        depart,
        move,
        unknown
    };

    PresenceRequest();
    PresenceRequest( enum PresenceOperation presenceOperation,
                     const ScenePresence& scenePresence,
                     const Coordinates& coordinates,
                     Direction direction = Direction::none,
                     bool push = false,
                     bool keyboard = false );

    void toTuple( Linda2::Tuple& tuple ) const;
    static PresenceRequest fromTuple( const Linda2::Tuple& tuple );

    virtual bool encrypt( Encryptor& encryptor );
    virtual bool decrypt( Encryptor& encryptor );

    PresenceOperation m_presenceOperation;
    ScenePresence m_scenePresence;
    Coordinates m_coordinates;
    Direction m_direction;
    bool m_push;

    String m_originatorID; // Set only for responses.

    bool m_keyboard; // Set only for manually generated requests.

private:
    static char presenceOperationToChar( enum PresenceOperation operation );
    static enum PresenceOperation presenceOperationFromChar( char operation );
    static String presenceOperationToString( enum PresenceOperation operation );
    static enum PresenceOperation presenceOperationFromString( const String& operation );
};

} // namespace Agape

#endif // AGAPE_PRESENCE_REQUEST_H
