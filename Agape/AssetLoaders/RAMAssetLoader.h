#ifndef AGAPE_ASSET_LOADERS_RAM_H
#define AGAPE_ASSET_LOADERS_RAM_H

#include "AssetLoader.h"
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

class Factory;

class RAM : public AssetLoader
{
public:
    RAM( const World::Coordinates& coordinates, const String& name, Map< String, String >& ramAssets, AssetLoaders::Factory& assetLoaderFactory );
    virtual ~RAM();

    virtual bool open();
    virtual bool open( enum OpenMode openMode, const String& linkedItem );
    virtual int read( char* data, int offset, int len );
    virtual int write( const char* data, int offset, int len );
    virtual bool close();
    virtual int size();

private:
    AssetLoaders::Factory& m_assetLoaderFactory;

    Map< String, String >& m_ramAssets;

    AssetLoader* m_assetLoader;
    bool m_ramAsset;
    enum OpenMode m_openMode;

    String m_writeBuffer;
};

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_RAM_H
