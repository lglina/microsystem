#ifndef AGAPE_ASSET_LOADERS_FACTORIES_MINI_MAP_H
#define AGAPE_ASSET_LOADERS_FACTORIES_MINI_MAP_H

#include "AssetLoaders/MiniMapAssetLoader.h"
#include "AssetLoadersFactory.h"
#include "String.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

class AssetLoader;
class String;

namespace AssetLoaders
{

namespace Factories
{

class MiniMap : public Factory
{
public:
    MiniMap( Agape::MiniMap& miniMap );

    virtual AssetLoader* makeLoader( const World::Coordinates& coordinates, const String& name );

private:
    Agape::MiniMap& m_miniMap;
};

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_FACTORIES_MINI_MAP_H
