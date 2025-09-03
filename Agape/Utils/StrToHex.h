#ifndef AGAPE_UTILS_STR_TO_HEX_H
#define AGAPE_UTILS_STR_TO_HEX_H

#include "String.h"

namespace Agape
{

String strToHex( const String& s );
String ullToHex( unsigned long long ull );
String ulToHex( unsigned long ul );
String uintToHex( unsigned int ui );
String ucharToHex( unsigned char uc );

String toHexStr( const char* data, int len );
void hexDump( const char* data, int len );

unsigned long long hexToUll( const String& s );

char nybbleToHexChar( int i );
int hexCharToNybble( char c );

} // namespace Agape

#endif // AGAPE_UTILS_STR_TO_HEX_H
