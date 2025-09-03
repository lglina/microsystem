#ifndef AGAPE_ASSET_LOADERS_FACTORY_H
#define AGAPE_ASSET_LOADERS_FACTORY_H

#include "AssetLoaders/AssetLoader.h"
#include "String.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

namespace AssetLoaders
{

class Factory
{
public:
    virtual ~Factory() {}
    
    virtual AssetLoader* makeLoader( const World::Coordinates& coordinates, const String& name ) = 0;
};

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_FACTORY_H
