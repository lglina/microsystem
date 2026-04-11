#ifndef AGAPE_ASSET_LOADERS_CALIGA_H
#define AGAPE_ASSET_LOADERS_CALIGA_H

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

class Caliga : public AssetLoader
{
public:
    Caliga( const World::Coordinates& coordinates, const String& name );
    virtual bool open();
    virtual int read( char* data, int offset, int len );
    virtual int size();

private:
    int m_size;
    const unsigned char* m_data;
};

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_CALIGA_H
