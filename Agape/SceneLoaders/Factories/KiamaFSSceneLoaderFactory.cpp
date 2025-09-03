#include "SceneLoaders/KiamaFSSceneLoader.h"
#include "SceneLoaders/SceneLoader.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "KiamaFS.h"
#include "String.h"
#include "KiamaFSSceneLoaderFactory.h"

namespace Agape
{

namespace SceneLoaders
{

namespace Factories
{

KiamaFS::KiamaFS( Agape::KiamaFS& fs ) :
  m_fs( fs )
{
    //m_fs.getIndex( m_index );
}

SceneLoader* KiamaFS::makeLoader( const World::Coordinates& coordinates, bool )
{
    return new SceneLoaders::KiamaFS( coordinates, m_fs, m_index );
}

} // namespace Factories

} // namespace SceneLoaders

} // namespace Agape
