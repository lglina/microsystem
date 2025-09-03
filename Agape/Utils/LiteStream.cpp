#include "LiteStream.h"
#include "String.h"
#include "Utils/printf.h"

void _putchar( char character )
{
    // STUB
}

namespace Agape
{

LiteStream& LiteStream::operator<<( const String& str )
{
    m_stream += str;
    return *this;
}

LiteStream& LiteStream::operator<<( int i )
{
    char s[12];
    ::snprintf( s, 12, "%d", i );
    m_stream += s;
    return *this;
}

LiteStream& LiteStream::operator<<( long l )
{
    char s[21];
    ::snprintf( s, 21, "%ld", l );
    m_stream += s;
    return *this;
}

LiteStream& LiteStream::operator<<( long long ll )
{
    char s[21];
    ::snprintf( s, 21, "%lld", ll );
    m_stream += s;
    return *this;
}

LiteStream& LiteStream::operator<<( unsigned int ui )
{
    char s[12];
    ::snprintf( s, 12, "%u", ui );
    m_stream += s;
    return *this;
}

LiteStream& LiteStream::operator<<( unsigned long ul )
{
    char s[21];
    ::snprintf( s, 21, "%lu", ul );
    m_stream += s;
    return *this;
}

LiteStream& LiteStream::operator<<( double f )
{
    char s[15];
    ::snprintf( s, 15, "%f", f );
    m_stream += s;
    return *this;
}

String LiteStream::str() const
{
    return m_stream;
}

LiteStream::operator const char*() const
{
    return m_stream.c_str();
}

}
