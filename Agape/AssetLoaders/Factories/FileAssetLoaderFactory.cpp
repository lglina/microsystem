#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/FileAssetLoader.h"
#include "World/WorldCoordinates.h"
#include "FileAssetLoaderFactory.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{

namespace Factories
{

File::File( const String& assetPath, const String& extension ) :
  m_assetPath( assetPath ),
  m_extension( extension )
{
}

AssetLoader* File::makeLoader( const World::Coordinates& coordinates, const String& name )
{
    return new AssetLoaders::File( coordinates, name, m_assetPath, m_extension );
}

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape
