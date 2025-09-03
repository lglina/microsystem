#ifndef AGAPE_WORLD_SCENE_H
#define AGAPE_WORLD_SCENE_H

#include "Collections.h"
#include "EncryptableDecryptable.h"
#include "SceneItem.h"
#include "WorldCoordinates.h"

namespace Agape
{

class Value;

namespace World
{

class Scene : public EncryptableDecryptable
{
public:
    Scene();

    void toValue( Value& value ) const;
    static Scene fromValue( const Value& value );

    int maxItems() const;

    virtual bool encrypt( Encryptor& encryptor );
    virtual bool decrypt( Encryptor& encryptor );

    World::Coordinates m_coordinates;
    Vector< SceneItem > m_sceneItems;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_WORLD_SCENE_H
