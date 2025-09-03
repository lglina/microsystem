#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Utils/Tokeniser.h"
#include "World/MiniMap.h"
#include "World/WorldCoordinates.h"
#include "MiniMapAssetLoader.h"
#include "String.h"

#include <stddef.h>
#include <stdlib.h>

namespace Agape
{

using namespace World;

namespace AssetLoaders
{

MiniMap::MiniMap( const Coordinates& coordinates,
                  const String& name,
                  World::MiniMap& miniMap ) :
  AssetLoader( coordinates, name ),
  m_miniMap( miniMap )
{
}

bool MiniMap::open()
{
    Tokeniser coordsTilesTokeniser( m_name, 'T' ); // Separate coords from tiling
    String assetNameCoords( coordsTilesTokeniser.token() );
    String assetNameTiles( coordsTilesTokeniser.token() );

    Tokeniser tilesTokeniser( assetNameTiles, '_' );
    int xtiles( ::atoi( tilesTokeniser.token().c_str() ) );
    int ytiles( ::atoi( tilesTokeniser.token().c_str() ) );
    m_miniMap.render( m_coordinates, xtiles, ytiles );
    return true;
}

int MiniMap::read( char* data, int offset, int len )
{
    return m_miniMap.read( data, offset, len );
}

int MiniMap::size()
{
    return m_miniMap.renderedSize();
}

} // namespace AssetLoaders

} // namespace Agape
