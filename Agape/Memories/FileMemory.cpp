#include "FileMemory.h"
#include "String.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

namespace Agape
{

namespace Memories
{

File::File( const String& filename, enum Type type, long createSize, long sectorSize ) :
  m_type( type ),
  m_sectorSize( sectorSize )
{
    struct stat sb;
    if( ::stat( filename.c_str(), &sb ) == -1 )
    {
#ifndef _WIN32
        m_fd = ::open( filename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
#else
        m_fd = ::open( filename.c_str(), O_RDWR | O_BINARY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
#endif

        char* emptySector = new char[ sectorSize ];
        ::memset( emptySector, '\xFF', sectorSize );

        for( long i = 0; i < createSize; i += sectorSize )
        {
            ::write( m_fd, emptySector, sectorSize );
        }
        m_size = createSize;

        delete[]( emptySector );
    }
    else
    {
#ifndef _WIN32
        m_fd = ::open( filename.c_str(), O_RDWR );
#else
        m_fd = ::open( filename.c_str(), O_RDWR | O_BINARY );
#endif
        m_size = sb.st_size;
    }
}

File::~File()
{
    ::close( m_fd );
}

enum Memory::Type File::type()
{
    return m_type;
}

int File::read( int addr, char* data, int len )
{
    if( addr + len > m_size )
    {
        std::cerr << "Read error: Attempt to read beyond end of memory." << std::endl;
        exit( 1 );
    }

    //std::cerr << "Reading " << len << " bytes at address " << addr << std::endl;
    ::lseek( m_fd, addr, SEEK_SET );

    for( int i = 0; i < len; ++i )
    {
        ::read( m_fd, data + i, 1 );
    }
    return len;
}

int File::write( int addr, const char* data, int len )
{
    if( addr + len > m_size )
    {
        std::cerr << "Write error: Attempt to write beyond end of memory." << std::endl;
        exit( 1 );
    }

    //std::cerr << "Writing " << len << " bytes at address " << addr << std::endl;
    ::lseek( m_fd, addr, SEEK_SET );
    
    int i = 0;
    for( ; i < len; ++i )
    {
        if( m_type == flash )
        {
            char c;
            ::read( m_fd, &c, 1 );
            if( c != '\xFF' )
            {
                std::cerr << "Write error: Attempt to write to memory not erased at offset " << addr + i << "." << std::endl;
                exit( 1 );
            }

            ::lseek( m_fd, -1, SEEK_CUR );
        }

        ::write( m_fd, data + i, 1 );
    }

    return i;
}

bool File::erase( int addr, int len )
{
    if( addr + len > m_size )
    {
        std::cerr << "Erase error: Attempt to erase beyond end of memory." << std::endl;
        exit( 1 );
    }

    if( m_type == flash )
    {
        if( addr % m_sectorSize != 0 )
        {
            std::cerr << "Erase error: Attempt to erase at address not aligned to sector boundary." << std::endl;
            exit( 1 );
        }
        
        if( len % m_sectorSize != 0 )
        {
            std::cerr << "Erase error: Attempt to erase length that is not a multiple of the sector size." << std::endl;
            exit( 1 );
        }
    }

    ::lseek( m_fd, addr, SEEK_SET );
    for( int i = 0; i < len; ++i )
    {
        ::write( m_fd, "\xFF", 1 );
    }

    return true;
}

int File::size()
{
    return m_size;
}

int File::sectorSize()
{
    return m_sectorSize;
}

} // namespace Memories

} // namespace Agape
