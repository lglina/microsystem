#ifndef AGAPE_SCENE_LOADER_H
#define AGAPE_SCENE_LOADER_H

#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "SceneRequest.h"

using namespace Agape::World;

namespace Agape
{

namespace World
{
class Scene;
} // namespace World

class SceneLoader
{
public:
    struct InvalidatedAsset
    {
        String m_name;
        String m_type;
        String m_originatorID;
    };

    static bool noReceiveRequests;

    SceneLoader( const Coordinates& coordinates );
    virtual ~SceneLoader();

    virtual bool load( Scene& scene ) = 0;
    virtual bool request( const Vector< SceneRequest >& requests ) = 0;
    virtual Vector< SceneRequest > getUpdates() = 0;
    
    virtual bool overflowed() { return false; }; // If implemented, should clear flag on retrieve if set.

    virtual bool hasSceneItemAttribute( const String& snowflake,
                                        const String& name ) { return false; };
    virtual bool createSceneItemAttribute( const String& snowflake,
                                           const String& name ) { return false; };
    virtual bool loadSceneItemAttribute( const String& snowflake,
                                         const String& name,
                                         Value& value ) { return false; };
    virtual bool saveSceneItemAttribute( const String& snowflake,
                                         const String& name,
                                         const Value& value ) { return false; };
    virtual bool deleteSceneItemAttributes( const String& snowflake ) { return false; };

    virtual void invalidateCachedAsset( const struct InvalidatedAsset& invalidatedAsset ) {};
    virtual Vector< struct InvalidatedAsset > getInvalidatedAssets();

protected:
    const Coordinates m_coordinates;
};

} // namespace Agape

#endif // AGAPE_SCENE_LOADER_H
