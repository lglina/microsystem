#ifndef AGAPE_STRING_H
#define AGAPE_STRING_H

#ifdef __WATCOMC__
#define throw
#define try if(true)
#define catch(...) if(false)
#endif

#include "Allocator.h"

#include <string>
#include <stddef.h>

namespace Agape
{

typedef std::basic_string< char, std::char_traits< char >, Allocator< char > > StringBase;

class String : public StringBase
{
public:
    String();
    String( size_t len, char c );
    String( const char* s );
    String( const char* s, size_t len );
    String( const StringBase& s );
    void pop_back() { erase(end()-1); }

#ifdef __WATCOMC__
    char back() { return operator[]( size() - 1 ); }
    friend bool operator==( const String& lhs, const String& rhs ) { return( (StringBase)lhs == (StringBase)rhs ); }
    friend bool operator!=( const String& lhs, const String& rhs ) { return( (StringBase)lhs != (StringBase)rhs ); }
    friend String operator+( const String& lhs, const String& rhs ) { return( (StringBase)lhs + (StringBase)rhs ); }
    friend String operator+( const String& lhs, char* rhs ) { return( (StringBase)lhs + (StringBase)rhs ); }
    friend bool operator<( const String& lhs, const String& rhs ) { return( (StringBase)lhs < (StringBase)rhs ); }
#endif
};

} // namespace Agape

#endif // AGAPE_NO_THROW_STRING_H
