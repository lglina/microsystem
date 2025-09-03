#ifndef AGAPE_SCENE_LOADERS_FACTORY_H
#define AGAPE_SCENE_LOADERS_FACTORY_H

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

class SceneLoader;

namespace SceneLoaders
{

class Factory
{
public:
    virtual ~Factory() {}
    
    virtual SceneLoader* makeLoader( const World::Coordinates& coordinates, bool receiveRequests = true ) = 0;
};

} // namespace SceneLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_FACTORY_H
