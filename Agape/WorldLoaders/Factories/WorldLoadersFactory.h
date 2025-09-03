#ifndef AGAPE_WORLD_LOADERS_FACTORY_H
#define AGAPE_WORLD_LOADERS_FACTORY_H

#include "WorldLoaders/WorldLoader.h"

namespace Agape
{

namespace WorldLoaders
{

class Factory
{
public:
    virtual ~Factory() {}
    
    virtual WorldLoader* makeLoader() = 0;
};

} // namespace WorldLoaders

} // namespace Agape

#endif // AGAPE_WORLD_LOADERS_FACTORY_H
