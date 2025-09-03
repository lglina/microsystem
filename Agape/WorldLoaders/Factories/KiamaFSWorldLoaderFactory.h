#ifndef AGAPE_WORLD_LOADERS_FACTORIES_KIAMA_FS_H
#define AGAPE_WORLD_LOADERS_FACTORIES_KIAMA_FS_H

#include "WorldLoadersFactory.h"

namespace Agape
{

class KiamaFS;

namespace WorldLoaders
{

namespace Factories
{

class KiamaFS : public Factory
{
public:
    KiamaFS( Agape::KiamaFS& fs );

    virtual WorldLoader* makeLoader();

private:
    Agape::KiamaFS& m_fs;
};

} // namespace Factories

} // namespace WorldLoaders

} // namespace Agape

#endif // AGAPE_WORLD_LOADERS_FACTORIES_KIAMA_FS_H
