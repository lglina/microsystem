#ifndef AGAPE_SCENE_REQUEST_H
#define AGAPE_SCENE_REQUEST_H

#include "World/Direction.h"
#include "World/SceneItem.h"
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

class SceneRequest : public EncryptableDecryptable
{
public:
    enum SceneOperation
    {
        create,
        update,
        remove,
        transport,
        raise,
        lower,
        unknown
    };

    SceneRequest();
    SceneRequest( enum SceneOperation sceneOperation,
                  const SceneItem& sceneItem,
                  const Coordinates& coordinates,
                  Direction direction = Direction::none,
                  bool keyboard = false );
    SceneRequest( const SceneItem& sceneItem,
                  const Coordinates& coordinates,
                  const Coordinates& newCoordinates );

    void toTuple( Linda2::Tuple& tuple ) const;
    static SceneRequest fromTuple( const Linda2::Tuple& tuple );

    virtual bool encrypt( Encryptor& encryptor );
    virtual bool decrypt( Encryptor& encryptor );

    bool operator<( const SceneRequest& other ) const;

    enum SceneOperation m_sceneOperation;
    SceneItem m_sceneItem;
    Coordinates m_coordinates;
    Coordinates m_newCoordinates;
    Direction m_direction;
    
    String m_originatorID; // Set only for responses.

    bool m_keyboard; // Set only for manually generated requests.

private:
    static char sceneOperationToChar( enum SceneOperation operation );
    static enum SceneOperation sceneOperationFromChar( char operation );
    static String sceneOperationToString( enum SceneOperation operation );
    static enum SceneOperation sceneOperationFromString( const String& operation );
};

} // namespace Agape

#endif // AGAPE_SCENE_REQUEST_H
