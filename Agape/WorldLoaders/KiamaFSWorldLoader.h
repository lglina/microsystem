#ifndef AGAPE_WORLD_LOADERS_KIAMA_FS_H
#define AGAPE_WORLD_LOADERS_KIAMA_FS_H

#include "String.h"
#include "WorldLoader.h"

namespace Agape
{

namespace World
{
class Metadata;
} // namespace World

class KiamaFS;

namespace WorldLoaders
{

class KiamaFS : public WorldLoader
{
public:
    KiamaFS( Agape::KiamaFS& fs );

    virtual bool create( const World::Metadata& metadata, String& reason );
    virtual bool join( World::Metadata& metadata, String& reason );
    virtual bool load( World::Metadata& metadata, String& reason );

private:
    Agape::KiamaFS& m_fs;
};

} // namespace WorldLoaders

} // namespace Agape

#endif // AGAPE_WORLD_LOADERS_KIAMA_FS_H
