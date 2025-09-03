#ifndef AGAPE_SCENE_LOADERS_MONGO_H
#define AGAPE_SCENE_LOADERS_MONGO_H

#include "Collections.h"
#include "SceneLoader.h"
#include "SceneRequest.h"
#include "World/Scene.h"

namespace Agape
{

namespace Stratus
{
class Authenticator;
} // namespace Stratus

namespace World
{
class Coordinates;
} // namespace World

using namespace Stratus;

namespace SceneLoaders
{

class Mongo : public SceneLoader
{
public:
    Mongo( const World::Coordinates& coordinates,
           Authenticator& authenticator );

    virtual bool load( World::Scene& scene );
    virtual bool request( const Vector< SceneRequest >& requests );
    virtual Vector< SceneRequest > getUpdates();

    virtual bool hasSceneItemAttribute( const String& snowflake,
                                        const String& name );
    virtual bool createSceneItemAttribute( const String& snowflake,
                                           const String& name );
    virtual bool loadSceneItemAttribute( const String& snowflake,
                                         const String& name,
                                         Value& value );
    virtual bool saveSceneItemAttribute( const String& snowflake,
                                         const String& name,
                                         const Value& value );
    virtual bool deleteSceneItemAttributes( const String& snowflake );

private:
    bool handleRequest( const SceneRequest& request );

    bool canCreate( const Scene& scene, const SceneItem& sceneItem );
    bool canUpdate( const Scene& scene, const SceneItem& sceneItem );
    bool canRemove( const Scene& scene, const SceneItem& sceneItem );

    Authenticator& m_authenticator;
};

} // namespace SceneLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_MONGO_H
