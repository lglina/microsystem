#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/KiamaFSAssetLoader.h"
#include "World/WorldCoordinates.h"
#include "KiamaFS.h"
#include "KiamaFSAssetLoaderFactory.h"

#include "Loggers/Logger.h"

namespace Agape
{

namespace AssetLoaders
{

namespace Factories
{

KiamaFS::KiamaFS( Agape::KiamaFS& fs, const String& extension ) :
  m_fs( fs ),
  m_extension( extension )
{
    //m_fs.getIndex( m_index );
}

AssetLoader* KiamaFS::makeLoader( const World::Coordinates& coordinates, const String& name )
{
	LOG_DEBUG( "Creating KiamaFS loader for " + name + "." + m_extension );
  // FIXME: For testing, re-scan for new asset files every time.
  //m_fs.getIndex( m_index );
  return new AssetLoaders::KiamaFS( coordinates, name, m_extension, m_fs, m_index );
}

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape
