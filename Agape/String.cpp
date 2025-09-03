#include "Allocator.h"
#include "String.h"

namespace Agape
{

String::String()
{
}

String::String( size_t len, char c ) :
  StringBase( len, c )
{
}

String::String( const char* s ) :
  StringBase( s )
{
}

String::String( const char* s, size_t len ) :
  StringBase( s, len )
{
}

String::String( const StringBase& s ) :
  StringBase( s )
{
}

} // namespace Agape
