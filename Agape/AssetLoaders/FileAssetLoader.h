#ifndef AGAPE_ASSET_LOADERS_FILE_H
#define AGAPE_ASSET_LOADERS_FILE_H

#include "AssetLoader.h"
#include "ReadableWritable.h"
#include "String.h"
#include "Value.h"

#include <fstream>
#include <sys/types.h>

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

namespace AssetLoaders
{

class File : public AssetLoader
{
public:
    File( const World::Coordinates& coordinates, const String& name, const String& assetPath, const String& extension );
    ~File();

    virtual bool open();
    virtual bool open( enum OpenMode openMode, const String& linkedItem );
    virtual int read( char* data, int offset, int len );
    virtual int write( const char* data, int offset, int len );
    virtual bool close();
    virtual int size();

    virtual bool move( const String& newName );
    virtual bool erase();

private:
    const String m_assetPath;
    const String m_extension;

    std::fstream* m_fstream;
    off_t m_size;

    Value m_attributes; // Will be map type.
};

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_FILE_H
