#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "SceneLoader.h"

using namespace Agape::World;

namespace Agape
{

bool SceneLoader::noReceiveRequests( false );

SceneLoader::SceneLoader( const Coordinates& coordinates ) :
  m_coordinates( coordinates )
{
}

SceneLoader::~SceneLoader()
{
}

Vector< struct SceneLoader::InvalidatedAsset > SceneLoader::getInvalidatedAssets()
{
    return Vector< struct InvalidatedAsset >();
}

} // namespace Agape
