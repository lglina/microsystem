#ifndef AGAPE_SCENE_LOADERS_KIAMA_FS_H
#define AGAPE_SCENE_LOADERS_KIAMA_FS_H

#include "Collections.h"
#include "KiamaFS.h"
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

class KiamaFS : public SceneLoader
{
public:
    KiamaFS( const World::Coordinates& coordinates, Agape::KiamaFS& fs, Map< String, int>& index );

    virtual bool load( World::Scene& scene );
    virtual bool request( const Vector< SceneRequest >& requests );
    virtual Vector< SceneRequest > getUpdates();

private:
    void handleRequest( const SceneRequest& request );

    Agape::KiamaFS& m_fs;
    Map< String, int>& m_index;
    bool m_inIndex;

    Vector< SceneRequest > m_updateLoopback;
};

} // namespace SceneLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_KIAMA_FS_H
