#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/RAMAssetLoader.h"
#include "AssetLoadersFactory.h"
#include "World/WorldCoordinates.h"
#include "RAMAssetLoaderFactory.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{

namespace Factories
{

RAM::RAM( AssetLoaders::Factory& assetLoaderFactory ) :
  m_assetLoaderFactory( assetLoaderFactory )
{
}

AssetLoader* RAM::makeLoader( const World::Coordinates& coordinates, const String& name )
{
    return new AssetLoaders::RAM( coordinates, name, m_ramAssets, m_assetLoaderFactory );
}

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape
