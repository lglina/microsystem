#include "SceneLoaders/MongoSceneLoader.h"
#include "SceneLoaders/SceneLoader.h"
#include "World/WorldCoordinates.h"
#include "Authenticator.h"
#include "MongoSceneLoaderFactory.h"

using namespace Agape::Stratus;

namespace Agape
{

namespace SceneLoaders
{

namespace Factories
{

Mongo::Mongo( Authenticator& authenticator ) :
  m_authenticator( authenticator )
{
}

SceneLoader* Mongo::makeLoader( const World::Coordinates& coordinates, bool )
{
    return new SceneLoaders::Mongo( coordinates, m_authenticator );
}

} // namespace Factories

} // namespace SceneLoaders

} // namespace Agape
