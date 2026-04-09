#include "String.h"
#include "StringSerialiser.h"

#include <string.h>

namespace Agape
{

StringSerialiser::StringSerialiser() :
  m_offset( 0 )
{
}

int StringSerialiser::read( char* data, int len )
{
    if( ( m_offset + len ) <= m_data.length() )
    {
        ::memcpy( data, &m_data[m_offset], len );
        m_offset += len;
        return len;
    }

    return 0;
}

int StringSerialiser::write( const char* data, int len )
{
    if( ( m_offset + len ) > m_data.length() ) m_data.resize( m_offset + len );
    ::memcpy( &m_data[m_offset], data, len );
    m_offset += len;
    return len;
}

bool StringSerialiser::error()
{
    return false;
}

} // namespace Agape
