#ifndef AGAPE_ASSET_LOADERS_FACTORIES_KIAMA_FS_H
#define AGAPE_ASSET_LOADERS_FACTORIES_KIAMA_FS_H

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

class KiamaFS;

namespace AssetLoaders
{

namespace Factories
{

class KiamaFS : public Factory
{
public:
    KiamaFS( Agape::KiamaFS& fs, const String& extension );

    virtual AssetLoader* makeLoader( const World::Coordinates& coordinates, const String& name );

private:
    Agape::KiamaFS& m_fs;
    String m_extension;
    Map< String, int > m_index;
};

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_FACTORIES_KIAMA_FS_H
