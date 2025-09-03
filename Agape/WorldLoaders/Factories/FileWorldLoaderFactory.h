#ifndef AGAPE_WORLD_LOADERS_FACTORIES_FILE_H
#define AGAPE_WORLD_LOADERS_FACTORIES_FILE_H

#include "WorldLoadersFactory.h"

namespace Agape
{

namespace WorldLoaders
{

namespace Factories
{

class File : public Factory
{
public:
    File( const String& path,
          const String& extension );

    virtual WorldLoader* makeLoader();

private:
    String m_path;
    String m_extension;
};

} // namespace Factories

} // namespace WorldLoaders

} // namespace Agape

#endif // AGAPE_WORLD_LOADERS_FACTORIES_FILE_H
