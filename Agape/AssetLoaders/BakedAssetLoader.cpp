#include "BakedAssetLoader.h"
#include "Assets/BakedAssets.h"
#include "World/WorldCoordinates.h"
#include "String.h"

#include <string.h>

namespace Agape
{

using namespace Assets::Baked;

namespace AssetLoaders
{

Baked::Baked( const World::Coordinates& coordinates, const String& name ) :
  AssetLoader( coordinates, name ),
  m_size( 0 ),
  m_data( nullptr )
{
}

bool Baked::open()
{
    for( int i = 0; i < metadataSize; ++i )
    {
        if( metadata[i].m_name == m_name )
        {
            m_size = metadata[i].m_size;
            m_data = metadata[i].m_data;
            return true;
        }
    }

    return false;
}

int Baked::read( char* data, int offset, int len )
{
    if( m_data == nullptr )
    {
        // Not yet opened!
        return 0;
    }

    int lengthRemaining( m_size - offset );
    int readLength( len <= lengthRemaining ? len : lengthRemaining );
    ::memcpy( data, m_data + offset, readLength );
    return readLength;
}

int Baked::size()
{
    if( m_data == nullptr )
    {
        // Not yet opened!
        return 0;
    }

    return m_size;
}

} // namespace AssetLoaders

} // namespace Agape
