#ifndef AGAPE_SCENE_LOADERS_FACTORIES_KIAMA_FS_H
#define AGAPE_SCENE_LOADERS_FACTORIES_KIAMA_FS_H

#include "SceneLoaders/SceneLoader.h"
#include "Collections.h"
#include "SceneLoadersFactory.h"
#include "String.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

class KiamaFS;

namespace SceneLoaders
{

namespace Factories
{

class KiamaFS : public Factory
{
public:
    KiamaFS( Agape::KiamaFS& fs );

    virtual SceneLoader* makeLoader( const World::Coordinates& coordinates, bool receiveRequests );

private:
    Agape::KiamaFS& m_fs;
    Map< String, int> m_index;
};

} // namespace Factories

} // namespace SceneLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_FACTORIES_KIAMA_FS_H
