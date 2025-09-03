#ifndef AGAPE_UTILS_TOKENISER_H
#define AGAPE_UTILS_TOKENISER_H

#include "String.h"
#include "stddef.h"

namespace Agape
{

class Tokeniser
{
public:
    Tokeniser( const String& string, char delim = '\n' );

    String token();
    String token( char delim );
    bool atEnd();
    String remainder();

private:
    const String& m_string;
    char m_delim;
    size_t m_from;
    bool m_atEnd;
};

} // namespace Agape

#endif // AGAPE_UTILS_TOKENISER_H
