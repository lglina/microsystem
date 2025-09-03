#ifndef AGAPE_SCENE_LOADERS_FACTORIES_FILE_H
#define AGAPE_SCENE_LOADERS_FACTORIES_FILE_H

#include "SceneLoaders/SceneLoader.h"
#include "SceneLoadersFactory.h"
#include "String.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

namespace SceneLoaders
{

namespace Factories
{

class File : public Factory
{
public:
    File( const String& worldPath,
          const String& worldExtension,
          const String& attributesPath,
          const String& attributesExtension );

    virtual SceneLoader* makeLoader( const World::Coordinates& coordinates, bool receiveRequests );

private:
    String m_worldPath;
    String m_worldExtension;
    String m_attributesPath;
    String m_attributesExtension;
};

} // namespace Factories

} // namespace SceneLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_FACTORIES_FILE_H
