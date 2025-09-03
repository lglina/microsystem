#ifndef AGAPE_ASSET_LOADERS_FACTORIES_BAKED_H
#define AGAPE_ASSET_LOADERS_FACTORIES_BAKED_H

#include "AssetLoaders/AssetLoader.h"
#include "AssetLoadersFactory.h"
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

class Baked : public Factory
{
public:
    Baked();

    virtual AssetLoader* makeLoader( const World::Coordinates& coordinates, const String& name );
};

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_FACTORIES_BAKED_H
