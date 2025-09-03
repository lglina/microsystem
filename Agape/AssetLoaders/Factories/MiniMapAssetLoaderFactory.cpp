#include "AssetLoaders/MiniMapAssetLoader.h"
#include "World/WorldCoordinates.h"
#include "MiniMapAssetLoaderFactory.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{

namespace Factories
{

MiniMap::MiniMap( Agape::MiniMap& miniMap ) :
  m_miniMap( miniMap )
{
}

AssetLoader* MiniMap::makeLoader( const World::Coordinates& coordinates, const String& name )
{
    return new AssetLoaders::MiniMap( coordinates,
                                      name,
                                      m_miniMap );
}

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape
