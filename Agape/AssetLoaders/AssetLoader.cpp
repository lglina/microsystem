#include "World/WorldCoordinates.h"
#include "AssetLoader.h"
#include "String.h"

namespace Agape
{

AssetLoader::AssetLoader( const World::Coordinates& coordinates, const String& name ) :
  m_coordinates( coordinates ),
  m_name( name )
{
}

AssetLoader::~AssetLoader()
{
}

bool AssetLoader::open( enum OpenMode openMode, const String& linkedItem )
{
    return open();
}

} // namespace Agape
