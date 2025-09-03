#include "SceneLoaders/FileSceneLoader.h"
#include "World/WorldCoordinates.h"
#include "FileSceneLoaderFactory.h"
#include "SceneLoadersFactory.h"
#include "String.h"

namespace Agape
{

namespace SceneLoaders
{

namespace Factories
{

File::File( const String& worldPath,
            const String& worldExtension,
            const String& attributesPath,
            const String& attributesExtension ) :
  m_worldPath( worldPath ),
  m_worldExtension( worldExtension ),
  m_attributesPath( attributesPath ),
  m_attributesExtension( attributesExtension )
{
}

SceneLoader* File::makeLoader( const World::Coordinates& coordinates, bool )
{
    return new SceneLoaders::File( coordinates,
                                   m_worldPath,
                                   m_worldExtension,
                                   m_attributesPath,
                                   m_attributesExtension );
}

} // namespace Factories

} // namespace SceneLoaders

} // namespace Agape
