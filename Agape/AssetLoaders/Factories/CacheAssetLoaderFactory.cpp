#include "AssetLoaders/Caches/AssetCache.h"
#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/CacheAssetLoader.h"
#include "Clocks/Clock.h"
#include "World/WorldCoordinates.h"
#include "CacheAssetLoaderFactory.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{

namespace Factories
{
Cache::Cache( Factory& assetLoaderBackingFactory,
              AssetCache& assetCache,
              bool encryptName ) :
  m_assetLoaderBackingFactory( assetLoaderBackingFactory ),
  m_assetCache( assetCache ),
  m_encryptName( encryptName )
{
}

AssetLoader* Cache::makeLoader( const World::Coordinates& coordinates, const String& name )
{
    return new AssetLoaders::Cache( coordinates,
                                    name,
                                    m_assetLoaderBackingFactory,
                                    m_assetCache,
                                    m_encryptName );
}

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape
