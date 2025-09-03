#ifndef AGAPE_ASSET_LOADERS_FACTORIES_RAM_H
#define AGAPE_ASSET_LOADERS_FACTORIES_RAM_H

#include "AssetLoaders/AssetLoader.h"
#include "AssetLoadersFactory.h"
#include "Collections.h"
#include "String.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

namespace AssetLoaders
{

namespace Factories
{

class RAM : public Factory
{
public:
    RAM( AssetLoaders::Factory& assetLoaderFactory );

    virtual AssetLoader* makeLoader( const World::Coordinates& coordinates, const String& name );

private:
    Map< String, String > m_ramAssets;
    AssetLoaders::Factory& m_assetLoaderFactory;
};

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_FACTORIES_RAM_H
