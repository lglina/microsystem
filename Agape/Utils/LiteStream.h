#ifndef AGAPE_LITE_STREAM_H
#define AGAPE_LITE_STREAM_H

#include "String.h"

extern "C"
{
    void _putchar( char character );
}

namespace Agape
{

class LiteStream
{
public:
    LiteStream& operator<<( const String& str );
    LiteStream& operator<<( int i );
    LiteStream& operator<<( long l );
    LiteStream& operator<<( long long ll );
    LiteStream& operator<<( unsigned int ui );
    LiteStream& operator<<( unsigned long ul );
    LiteStream& operator<<( double f );

    String str() const;

    operator const char*() const;

private:
    String m_stream;
};

} // namespace Agape

#endif // AGAPE_LITE_STREAM_H
