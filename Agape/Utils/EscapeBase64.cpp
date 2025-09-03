#include "EscapeBase64.h"
#include "String.h"

namespace Agape
{

String escapeBase64( const String& str )
{
    String escaped( str );

    for( int i = 0; i < str.length(); ++i )
    {
        if( str[i] == '/' ) { escaped[i] = '_'; }
        if( str[i] == '+' ) { escaped[i] = '-'; }
    }

    return escaped;
}

} // namespace Agape
