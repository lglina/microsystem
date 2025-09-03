#ifndef AGAPE_ASSET_LOADER_H
#define AGAPE_ASSET_LOADER_H

#include "World/WorldCoordinates.h"
#include "RandomReadableWritable.h"
#include "String.h"

namespace Agape
{

class AssetLoader : public RandomReadableWritable
{
public:
    enum OpenMode
    {
        modeRead,
        modeWrite
    };

    AssetLoader( const World::Coordinates& coordinates, const String& name );
    virtual ~AssetLoader();

    virtual bool open() = 0;
    virtual bool open( enum OpenMode openMode, const String& linkedItem );
    virtual int read( char* data, int offset, int len ) = 0;
    virtual int write( const char* data, int offset, int len ) { return 0; };
    virtual bool close() { return true; };
    virtual int size() = 0;

    // Asset must not be open, or have been closed, prior to move or erase.
    virtual bool move( const String& newName ) { return false; };
    virtual bool erase() { return false; };

    virtual bool error() { return false; };

    virtual void invalidateCached( bool all = false ) {};

protected:
    const World::Coordinates m_coordinates;
    String m_name;
};

} // namespace Agape

#endif // AGAPE_ASSET_LOADER_H
