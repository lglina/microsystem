#ifndef AGAPE_ASSET_LOADERS_FACTORIES_FILE_H
#define AGAPE_ASSET_LOADERS_FACTORIES_FILE_H

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

class File : public Factory
{
public:
    File( const String& assetPath, const String& extension );

    virtual AssetLoader* makeLoader( const World::Coordinates& coordinates, const String& name );

private:
    String m_assetPath;
    String m_extension;
};

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_FACTORIES_FILE_H
