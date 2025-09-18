#ifndef AGAPE_ASSET_LOADERS_KIAMA_FS_H
#define AGAPE_ASSET_LOADERS_KIAMA_FS_H

#include "AssetLoaders/AssetLoader.h"
#include "Collections.h"
#include "KiamaFS.h"
#include "String.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

namespace AssetLoaders
{

class KiamaFS : public AssetLoader
{
public:
    KiamaFS( const World::Coordinates& coordinates,
             const String& name,
             const String& extension,
             Agape::KiamaFS& fs,
             Map< String, int>& index );
    ~KiamaFS();

    virtual bool open();
    virtual int read( char* data, int offset, int len );
    virtual int size();

private:
    String m_extension;
    Agape::KiamaFS& m_fs;
    Map< String, int > m_index;
    
    Agape::KiamaFS::File* m_file;
    int m_currentOffset;
};

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_KIAMA_FS_H
