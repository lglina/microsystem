#ifndef AGAPE_ASSET_LOADERS_MINI_MAP_H
#define AGAPE_ASSET_LOADERS_MINI_MAP_H

#include "AssetLoader.h"

namespace Agape
{

namespace World
{
class Coordinates;
class MiniMap;
} // namespace World

using namespace World;

namespace AssetLoaders
{

class MiniMap : public AssetLoader
{
public:
    MiniMap( const Coordinates& coordinates,
             const String& name,
             World::MiniMap& miniMap );
    virtual bool open();
    virtual int read( char* data, int offset, int len );
    virtual int size();

private:
    World::MiniMap& m_miniMap;
};

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_MINI_MAP_H
