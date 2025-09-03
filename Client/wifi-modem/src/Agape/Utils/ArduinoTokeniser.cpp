#include "ArduinoTokeniser.h"

#include <Arduino.h>

namespace Agape
{

ArduinoTokeniser::ArduinoTokeniser( const String& string, char delim ) :
  m_string( string ),
  m_delim( delim ),
  m_from( 0 )
{
}

String ArduinoTokeniser::token()
{
    return token( m_delim );
}

String ArduinoTokeniser::token( char delim )
{
    int nextDelim( m_string.indexOf( delim, m_from ) );
    if( nextDelim != -1 )
    {
        String token( m_string.substring( m_from, nextDelim ) );
        m_from = nextDelim + 1;
        return token;
    }
    else
    {
        String token( m_string.substring( m_from, m_string.length() ) );
        m_from = m_string.length();
        return token;
    }
}

} // namespace Agape
