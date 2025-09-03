#include "Tokeniser.h"

#include "String.h"

namespace Agape
{

Tokeniser::Tokeniser( const String& string, char delim ) :
  m_string( string ),
  m_delim( delim ),
  m_from( 0 ),
  m_atEnd( false )
{
}

String Tokeniser::token()
{
    return token( m_delim );
}

String Tokeniser::token( char delim )
{
    size_t nextDelim( m_string.find( delim, m_from ) );
    if( nextDelim != String::npos )
    {
        String token( m_string.substr( m_from, nextDelim - m_from ) );
        m_from = nextDelim + 1;
        return token;
    }
    else
    {
        String token( m_string.substr( m_from ) );
        m_from = m_string.size();
        m_atEnd = true;
        return token;
    }
}

bool Tokeniser::atEnd()
{
    return m_atEnd;
}

String Tokeniser::remainder()
{
    String token( m_string.substr( m_from ) );
    m_from = m_string.size();
    return token;
}

} // namespace Agape
