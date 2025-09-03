#include "Loggers/Logger.h"
#include "RWBuffer.h"

#include <string.h>

namespace Agape
{

RWBuffer::RWBuffer( int size, ReadableWritable& rw ) :
  m_size( size ),
  m_rw( rw ),
  m_offset( 0 )
{
    m_buffer = new char[ size ];
}

RWBuffer::~RWBuffer()
{
    delete[]( m_buffer );
}

int RWBuffer::read( char* data, int len )
{
    return( m_rw.read( data, len ) );
}

int RWBuffer::write( const char* data, int len )
{
    if( ( m_offset + len ) >= m_size )
    {
        flushOutput();
    }

    ::memcpy( m_buffer + m_offset, data, len );
    m_offset += len;

    return len;
}

bool RWBuffer::error()
{
    return m_rw.error();
}

void RWBuffer::flushOutput()
{
    int numWritten( 0 );
    int numThisWrite( -1 );
    while( !m_rw.error() &&
           ( numThisWrite != 0 ) &&
           ( numWritten < m_offset ) )
    {
        numThisWrite = m_rw.write( m_buffer + numWritten, ( m_offset - numWritten ) );
        numWritten += numThisWrite;
    }

    m_offset = 0;
}

} // namespace Agape
