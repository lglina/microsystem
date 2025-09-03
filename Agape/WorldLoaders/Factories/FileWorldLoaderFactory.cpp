#include "WorldLoaders/FileWorldLoader.h"
#include "FileWorldLoaderFactory.h"
#include "TupleRouter.h"

namespace Agape
{

namespace WorldLoaders
{

namespace Factories
{

File::File( const String& path,
            const String& extension ) :
  m_path( path ),
  m_extension( extension )
{
}

WorldLoader* File::makeLoader()
{
    return new WorldLoaders::File( m_path, m_extension );
}

} // namespace Factories

} // namespace WorldLoaders

} // namespace Agape
