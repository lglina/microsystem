#include "Asset.h"
#include "AssetLoaders/AssetLoader.h"

namespace Agape
{

Asset::Asset( AssetLoader& loader ) :
  m_loader( loader )
{
}

Asset::~Asset()
{
}

int Asset::read( char* data, int offset, int len ) const
{
    return m_loader.read( data, offset, len );
}

int Asset::write( const char* data, int offset, int len )
{
    return m_loader.write( data, offset, len );
}

int Asset::size()
{
    return m_loader.size();
}

int Asset::readAll( char* data, int offset, int len ) const
{
    return m_loader.readAll( data, offset, len );
}

} // namespace Agape
