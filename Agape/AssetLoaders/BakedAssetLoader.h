#ifndef AGAPE_ASSET_LOADERS_BAKED_H
#define AGAPE_ASSET_LOADERS_BAKED_H

#include "AssetLoader.h"
#include "String.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

namespace AssetLoaders
{

class Baked : public AssetLoader
{
public:
    Baked( const World::Coordinates& coordinates, const String& name );
    virtual bool open();
    virtual int read( char* data, int offset, int len );
    virtual int size();

private:
    int m_size;
    const unsigned char* m_data;
};

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_BAKED_H
