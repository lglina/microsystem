#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "RAMAssetLoader.h"
#include "String.h"

#include <string.h>

namespace Agape
{

namespace AssetLoaders
{

RAM::RAM( const World::Coordinates& coordinates, const String& name, Map< String, String >& ramAssets, AssetLoaders::Factory& assetLoaderFactory ) :
  AssetLoader( coordinates, name ),
  m_ramAssets( ramAssets ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_assetLoader( nullptr ),
  m_ramAsset( false )
{
}

RAM::~RAM()
{
    if( m_assetLoader )
    {
        delete( m_assetLoader );
    }
}

bool RAM::open()
{
    return( open( AssetLoader::modeRead, String() ) );
}

bool RAM::open( enum OpenMode openMode, const String& linkedItem )
{
    bool opened( false );
    m_openMode = openMode;

    if( openMode == AssetLoader::modeRead )
    {
        Map< String, String >::const_iterator ramAssetIt( m_ramAssets.find( m_name ) );
        if( ramAssetIt != m_ramAssets.end() )
        {
            m_ramAsset = true;
            opened = true;
        }

        if( !opened )
        {
            m_assetLoader = m_assetLoaderFactory.makeLoader( m_coordinates, m_name );
            if( m_assetLoader->open( openMode, linkedItem ) )
            {
                opened = true;
            }
        }
    }
    else if( openMode == AssetLoader::modeWrite )
    {
        m_ramAsset = true;
        opened = true;
    }

    return opened;
}

int RAM::read( char* data, int offset, int len )
{
    int numRead( 0 );

    if( m_ramAsset )
    {
        Map< String, String >::const_iterator ramAssetIt( m_ramAssets.find( m_name ) );
        if( ramAssetIt != m_ramAssets.end() )
        {
            const String& ramAsset( ramAssetIt->second );
            if( offset < ramAsset.size() )
            {
                int canRead( ramAsset.size() - offset );
                int numToRead( canRead >= len ? len : canRead );
                ::memcpy( data, &ramAsset[0] + offset, numToRead );
                numRead = numToRead;
            }
        }
    }
    else if( m_assetLoader )
    {
        numRead = m_assetLoader->read( data, offset, len );
    }

    return numRead;
}

int RAM::write( const char* data, int offset, int len )
{
    int numWritten( 0 );

    if( m_ramAsset )
    {
        if( m_writeBuffer.size() < ( offset + len ) )
        {
            m_writeBuffer.resize( ( offset + len ), '\0' );
        }
        m_writeBuffer.replace( offset, len, data, len );
        numWritten = len;
    }

    return numWritten;
}

bool RAM::close()
{
    if( m_ramAsset )
    {
        if( m_openMode == AssetLoader::modeWrite )
        {
            m_ramAssets[m_name] = m_writeBuffer;
        }
        return true;
    }
    else if( m_assetLoader )
    {
        return m_assetLoader->close();
    }

    return false;
}

int RAM::size()
{
    int size( 0 );

    if( m_ramAsset )
    {
        Map< String, String >::const_iterator ramAssetIt( m_ramAssets.find( m_name ) );
        if( ramAssetIt != m_ramAssets.end() )
        {
            size = ramAssetIt->second.length();
        }
    }
    else if( m_assetLoader )
    {
        size = m_assetLoader->size();
    }

    return size;
}

} // namespace AssetLoaders

} // namespace Agape
