#ifndef AGAPE_SCENE_LOADERS_FILE_H
#define AGAPE_SCENE_LOADERS_FILE_H

#include "World/Scene.h"
#include "Collections.h"
#include "SceneLoader.h"
#include "SceneRequest.h"
#include "String.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

namespace SceneLoaders
{

class File : public SceneLoader
{
public:
    File( const World::Coordinates& coordinates,
          const String& scenePath,
          const String& sceneExtension,
          const String& attributesPath,
          const String& attributesExtension );

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
    void save( const World::Scene& scene );

    bool loadSceneItemAttributesFile( const String& snowflake, Value& attributes );
    bool saveSceneItemAttributesFile( const String& snowflake, const Value& attributes );

    void handleRequest( const SceneRequest& request );

    String sceneFilename();
    String sceneItemAttributesFilename( const String& snowflake );

    Vector< SceneRequest > m_updateLoopback;
    String m_scenePath;
    String m_sceneExtension;
    String m_attributesPath;
    String m_attributesExtension;
};

} // namespace SceneLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_FILE_H
