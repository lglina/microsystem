#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/BakedAssetLoader.h"
#include "World/WorldCoordinates.h"
#include "BakedAssetLoaderFactory.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{

namespace Factories
{

Baked::Baked()
{
}

AssetLoader* Baked::makeLoader( const World::Coordinates& coordinates, const String& name )
{
    return new AssetLoaders::Baked( coordinates, name );
}

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape
