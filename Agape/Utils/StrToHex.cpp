#include "Loggers/Logger.h"
#include "String.h"
#include "StrToHex.h"

namespace Agape
{

String strToHex( const String& s )
{
    String hex;
    for( int i = 0; i < s.length(); ++i )
    {
        hex += nybbleToHexChar( *(unsigned char*)( &s[i] ) >> 4 );
        hex += nybbleToHexChar( ( *(unsigned char*)( &s[i]) ) & 0x0F );
    }

    return hex;
}

String ullToHex( unsigned long long ull )
{
    String hex;
    for( int i( ( sizeof( unsigned long long ) * 2 ) - 1 ); i >= 0; --i )
    {
        hex += nybbleToHexChar( ( ull >> ( i * 4 ) ) & 0xF );
    }

    return hex;
}

String ulToHex( unsigned long ul )
{
    String hex;
    for( int i( ( sizeof( unsigned long ) * 2 ) - 1 ); i >= 0; --i )
    {
        hex += nybbleToHexChar( ( ul >> ( i * 4 ) ) & 0xF );
    }

    return hex;
}

String uintToHex( unsigned int ui )
{
    String hex;
    for( int i( ( sizeof( unsigned int ) * 2 ) - 1 ); i >= 0; --i )
    {
        hex += nybbleToHexChar( ( ui >> ( i * 4 ) ) & 0xF );
    }

    return hex;
}

String ucharToHex( unsigned char uc )
{
    String hex;
    for( int i( ( sizeof( unsigned char ) * 2 ) - 1 ); i >= 0; --i )
    {
        hex += nybbleToHexChar( ( uc >> ( i * 4 ) ) & 0xF );
    }

    return hex;
}

String toHexStr( const char* data, int len )
{
    String line;
    String lines;
    for( int i = 0; i < len; ++i )
    {
        if( line.empty() )
        {
            line += uintToHex( i ) + "  ";
        }

        line += ucharToHex( ( (unsigned char*)data )[i] );

        if( ( i + 1 ) % 16 == 0 )
        {
            lines += line + "\n";
            line.clear();
        }
        else if( ( i + 1 ) % 8 == 0 )
        {
            line += "  ";
        }
        else
        {
            line += " ";
        }
    }

    if( !line.empty() )
    {
        lines += line + "\n";
    }

    return lines;
}

void hexDump( const char* data, int len )
{
    String lines( toHexStr( data, len ) );

    if( !lines.empty() )
    {
        lines.pop_back(); // Remove trailing newline - LOG_DEBUG adds its own.
        LOG_DEBUG( lines );
    }
}

unsigned long long hexToUll( const String& s )
{
    unsigned long long ull( 0 );
    for( int i = s.length() - 1; i >= 0; --i )
    {
        ull += (unsigned long long)hexCharToNybble( s[s.length() - i - 1] ) << ( i * 4 );
    }

    return ull;
}

char nybbleToHexChar( int i )
{
    if( i >= 0 && i <= 9 )
    {
        return '0' + i;
    }
    else if( i >= 10 && i <= 15 )
    {
        return 'A' + ( i - 10 );
    }
    else
    {
        return '0';
    }
}

int hexCharToNybble( char c )
{
    if( c >= '0' && c <= '9' )
    {
        return c - '0';
    }
    else if( c >= 'A' && c <= 'F' )
    {
        return 10 + ( c - 'A' );
    }
    else
    {
        return 0;
    }
}

} // namepsace Agape
