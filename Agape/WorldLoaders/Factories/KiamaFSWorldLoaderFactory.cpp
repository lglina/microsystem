#include "WorldLoaders/KiamaFSWorldLoader.h"
#include "KiamaFS.h"
#include "KiamaFSWorldLoaderFactory.h"
#include "TupleRouter.h"

namespace Agape
{

namespace WorldLoaders
{

namespace Factories
{

KiamaFS::KiamaFS( Agape::KiamaFS& fs ) :
  m_fs( fs )
{
}

WorldLoader* KiamaFS::makeLoader()
{
    return new WorldLoaders::KiamaFS( m_fs );
}

} // namespace Factories

} // namespace WorldLoaders

} // namespace Agape
