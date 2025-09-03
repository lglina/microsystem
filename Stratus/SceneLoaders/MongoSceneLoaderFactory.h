#ifndef AGAPE_SCENE_LOADERS_FACTORIES_MONGO_H
#define AGAPE_SCENE_LOADERS_FACTORIES_MONGO_H

#include "SceneLoaders/SceneLoader.h"
#include "SceneLoadersFactory.h"

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

namespace Factories
{

class Mongo : public Factory
{
public:
    Mongo( Authenticator& authenticator );

    virtual SceneLoader* makeLoader( const World::Coordinates& coordinates, bool receiveRequests );

private:
    Authenticator& m_authenticator;
};

} // namespace Factories

} // namespace SceneLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_FACTORIES_MONGO_H
