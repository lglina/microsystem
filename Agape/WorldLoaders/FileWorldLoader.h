#ifndef AGAPE_WORLD_LOADERS_FILE_H
#define AGAPE_WORLD_LOADERS_FILE_H

#include "String.h"
#include "WorldLoader.h"

namespace Agape
{

namespace World
{
class Metadata;
} // namespace World

namespace WorldLoaders
{

class File : public WorldLoader
{
public:
    File( const String& path,
          const String& extension );

    virtual bool create( const World::Metadata& metadata, String& reason );
    virtual bool join( World::Metadata& metadata, String& reason );
    virtual bool load( World::Metadata& metadata, String& reason );

private:
    String worldFilename( const String& worldID );

    String m_path;
    String m_extension;
};

} // namespace WorldLoaders

} // namespace Agape

#endif // AGAPE_WORLD_LOADERS_FILE_H
