#include "AssetLoaders/AssetLoader.h"
#include "RAMMemory.h"

//#include <iostream>
#include <string.h>

namespace Agape
{

namespace Memories
{

RAM::RAM( int size, int sectorSize, enum Type type ) :
  m_size( size ),
  m_sectorSize( sectorSize ),
  m_type( type ),
#ifdef NOEXCEPT
  m_data( new ( std::nothrow ) char[size] )
#else
  m_data( new char[size] )
#endif
{
    ::memset( m_data, '\xFF', size );
}

RAM::~RAM()
{
    delete[]( m_data );
}

enum Memory::Type RAM::type()
{
    return m_type;
}

int RAM::read( int addr, char* data, int len )
{
    if( addr + len > m_size )
    {
        //std::cerr << "Read error: Attempt to read beyond end of memory." << std::endl;
        return 0;
    }

    ::memcpy( data, m_data + addr, len );

    return len;
}

int RAM::write( int addr, const char* data, int len )
{
    if( addr + len > m_size )
    {
        //std::cerr << "Write error: Attempt to write beyond end of memory." << std::endl;
        return 0;
    }
    
    int i = 0;
    for( ; i < len; ++i )
    {
        if( m_type == flash )
        {
            char c( m_data[addr+i] );
            if( c != '\xFF' )
            {
                //std::cerr << "Write error: Attempt to write to memory not erased at offset " << addr + i << "." << std::endl;
                return i;
            }
        }

        m_data[addr+i] = *( data + i );
    }

    return i;
}

bool RAM::erase( int addr, int len )
{
    if( addr + len > m_size )
    {
        //std::cerr << "Erase error: Attempt to erase beyond end of memory." << std::endl;
        return false;
    }

    if( m_type == flash )
    {
        if( addr % m_sectorSize != 0 )
        {
            //std::cerr << "Erase error: Attempt to erase at address not aligned to sector boundary." << std::endl;
            return false;
        }
        
        if( len % m_sectorSize != 0 )
        {
            //std::cerr << "Erase error: Attempt to erase length that is not a multiple of the sector size." << std::endl;
            return false;
        }
    }

    ::memset( m_data + addr, '\xFF', len );

    return true;
}

int RAM::size()
{
    return m_size;
}

int RAM::sectorSize()
{
    return m_sectorSize;
}

void RAM::loadFromAsset( AssetLoader& assetLoader )
{
    const int blockSize( 128 );
    if( assetLoader.open() )
    {
        int bytesCopied( 0 );
        while( bytesCopied < assetLoader.size() )
        {
            int bytesRemaining( assetLoader.size() - bytesCopied );
            int bytesToCopy( bytesRemaining < blockSize ? bytesRemaining : blockSize );
            if( assetLoader.read( m_data + bytesCopied, bytesCopied, bytesToCopy ) != bytesToCopy )
            {
                break;
            }
            bytesCopied += bytesToCopy;
        }

        assetLoader.close();
    }
}

} // namespace Memories

} // namespace Agape
