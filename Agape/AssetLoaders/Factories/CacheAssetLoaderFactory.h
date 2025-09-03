#ifndef AGAPE_ASSET_LOADERS_FACTORIES_CACHE_H
#define AGAPE_ASSET_LOADERS_FACTORIES_CACHE_H

#include "AssetLoaders/CacheAssetLoader.h"
#include "AssetLoadersFactory.h"
#include "String.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

class AssetLoader;
class Clock;
class String;

namespace AssetLoaders
{

class AssetCache;

namespace Factories
{

class Cache : public Factory
{
public:
    Cache( Factory& assetLoaderBackingFactory,
           AssetCache& assetCache,
           bool encryptName );

    virtual AssetLoader* makeLoader( const World::Coordinates& coordinates, const String& name );

private:
    Factory& m_assetLoaderBackingFactory;
    AssetCache& m_assetCache;
    bool m_encryptName;
};

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_FACTORIES_CACHE_H
