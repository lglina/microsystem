#include "Memory.h"

namespace Agape
{

Memory::Memory() :
  m_rwAddress( 0 )
{
}

int Memory::read( char* data, int len )
{
    int numRead( read( m_rwAddress, data, len ) );
    m_rwAddress += numRead;
    return numRead;
}

int Memory::write( const char* data, int len )
{
    int numWritten( write( m_rwAddress, data, len ) );
    m_rwAddress += numWritten;
    return numWritten;
}

bool Memory::erase()
{
    return erase( 0, size() );
}

bool Memory::error()
{
    return false;
}

void Memory::seek( int offset )
{
    m_rwAddress = offset;
}

} // namespace Agape
