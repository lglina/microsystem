#ifndef AGAPE_CARLO_LEXER_H
#define AGAPE_CARLO_LEXER_H

#include "Collections.h"
#include "String.h"

namespace Agape
{

namespace Carlo
{

class Lexer
{
public:
    enum Code
    {
        l_none,
        l_string,
        l_quotedString,
        l_float,
        l_tuple,
        l_actor,
        l_receives,
        l_sends,
        l_to,
        l_on,
        l_from,
        l_as,
        l_where,
        l_if,
        l_else,
        l_end,
        l_plus,
        l_minus,
        l_asterisk,
        l_forwardSlash,
        l_percent,
        l_is,
        l_not,
        l_and,
        l_or,
        l_makes,
        l_creates,
        l_value,
        l_leftParen,
        l_rightParen,
        l_note,
        l_each,
        l_in,
        l_stop,
        l_more,
        l_less,
        l_than,
        l_with,
        l_while,
        l_eol
    };

    static char* s_string[];

    struct Token
    {
    public:
        Token() :
          m_code( l_none ),
          m_float( 0.0 ),
          m_line( 0 ),
          m_column( 0 ),
          m_len( 0 )
        {
        }
        
        Token( enum Code code, int line, int column, int len ) :
          m_code( code ),
          m_float( 0.0 ),
          m_line( line ),
          m_column( column ),
          m_len( len )
        {
        }

        Token( enum Code code, const String& _string, int line, int column, int len ) :
          m_code( code ),
          m_string( _string ),
          m_float( 0.0 ),
          m_line( line ),
          m_column( column ),
          m_len( len )
        {
        }

        Token( enum Code code, double _float, int line, int column, int len ) :
          m_code( code ),
          m_float( _float ),
          m_line( line ),
          m_column( column ),
          m_len( len )
        {
        }

        enum Code m_code;
        String m_string;
        double m_float;

        int m_line;
        int m_column;
        int m_len;
    };

    Lexer();

    void lex( const String& input, Deque< Token >& tokens );

private:
    int m_line;
};

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_LEXER_H
