#include "Lexer.h"

#include "Utils/Tokeniser.h"

#include "Collections.h"
#include "String.h"

#include <cstdlib>

namespace Agape
{

namespace Carlo
{

char* Lexer::s_string[] =
{
    "_none_",
    "_string_",
    "_quotedString_",
    "_float_",
    "tuple",
    "actor",
    "receives",
    "sends",
    "to",
    "on",
    "from",
    "as",
    "where",
    "if",
    "else",
    "end",
    "+",
    "-",
    "*",
    "/",
    "%",
    "is",
    "not",
    "and",
    "or",
    "makes",
    "creates",
    "value",
    "(",
    ")",
    "note",
    "each",
    "in",
    "stop",
    "more",
    "less",
    "than",
    "with",
    "while",
    "_eol_"
};

Lexer::Lexer() :
  m_line( 0 )
{
}

void Lexer::lex( const String& input, Deque< Token >& tokens )
{
    Tokeniser tokeniser( input, ' ' );
    String tokenStr;
    int tokenLen( 0 );
    int column( 0 );
    while( !tokeniser.atEnd() )
    {
        tokenStr = tokeniser.token();

        if( !tokenStr.empty() )
        {
            if( tokenStr.back() == '\n' ) tokenStr.pop_back();
        }

        if( !tokenStr.empty() )
        {
            if( tokenStr.back() == '\r' ) tokenStr.pop_back();
        }

        tokenLen = tokenStr.length();

        if( tokenStr == s_string[l_tuple] )
        {
            tokens.push_back( Token( l_tuple, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_actor] )
        {
            tokens.push_back( Token( l_actor, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_receives] )
        {
            tokens.push_back( Token( l_receives, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_sends] )
        {
            tokens.push_back( Token( l_sends, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_to] )
        {
            tokens.push_back( Token( l_to, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_on] )
        {
            tokens.push_back( Token( l_on, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_from] )
        {
            tokens.push_back( Token( l_from, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_as] )
        {
            tokens.push_back( Token( l_as, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_where] )
        {
            tokens.push_back( Token( l_where, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_if] )
        {
            tokens.push_back( Token( l_if, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_else] )
        {
            tokens.push_back( Token( l_else, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_end] )
        {
            tokens.push_back( Token( l_end, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_plus] )
        {
            tokens.push_back( Token( l_plus, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_minus] )
        {
            tokens.push_back( Token( l_minus, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_asterisk] )
        {
            tokens.push_back( Token( l_asterisk, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_forwardSlash] )
        {
            tokens.push_back( Token( l_forwardSlash, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_percent] )
        {
            tokens.push_back( Token( l_percent, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_is] )
        {
            tokens.push_back( Token( l_is, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_not] )
        {
            tokens.push_back( Token( l_not, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_and] )
        {
            tokens.push_back( Token( l_and, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_or] )
        {
            tokens.push_back( Token( l_or, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_makes] )
        {
            tokens.push_back( Token( l_makes, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_creates] )
        {
            tokens.push_back( Token( l_creates, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_value] )
        {
            tokens.push_back( Token( l_value, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_leftParen] )
        {
            tokens.push_back( Token( l_leftParen, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_rightParen] )
        {
            tokens.push_back( Token( l_rightParen, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_note] )
        {
            tokens.push_back( Token( l_note, m_line, column, input.length() - column ) );
            column = input.length();
            break;
        }
        else if( tokenStr == s_string[l_each] )
        {
            tokens.push_back( Token( l_each, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_in] )
        {
            tokens.push_back( Token( l_in, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_stop] )
        {
            tokens.push_back( Token( l_stop, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_more] )
        {
            tokens.push_back( Token( l_more, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_less] )
        {
            tokens.push_back( Token( l_less, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_than] )
        {
            tokens.push_back( Token( l_than, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_with] )
        {
            tokens.push_back( Token( l_with, m_line, column, tokenStr.length() ) );
        }
        else if( tokenStr == s_string[l_while] )
        {
            tokens.push_back( Token( l_while, m_line, column, tokenStr.length() ) );
        }
        else if( !tokenStr.empty() )
        {
            if( tokenStr[0] == '"' )
            {
                String quotedStr( tokenStr );
                while( ( tokenStr != "" ) && ( tokenStr[ tokenStr.size() - 1 ] != '"' ) )
                {
                    quotedStr += " ";
                    tokenStr = tokeniser.token();
                    quotedStr += tokenStr;
                }
                // FIXME: Detect missing closing quote?
                tokens.push_back( Token( l_quotedString, quotedStr.substr( 1, quotedStr.length() - 2 ), m_line, column, quotedStr.length() ) );
                tokenLen = quotedStr.length();
            }
            else if( tokenStr[0] <= 'z' && tokenStr[0] >= 'a' )
            {
                tokens.push_back( Token( l_string, tokenStr, m_line, column, tokenStr.length() ) );
            }
            else if( tokenStr[0] <= 'Z' && tokenStr[0] >= 'A' )
            {
                tokens.push_back( Token( l_string, tokenStr, m_line, column, tokenStr.length() ) );
            }
            else
            {
                // FIXME: Might return 0.0 for non-floats and malformed.
                tokens.push_back( Token( l_float, ::atof( tokenStr.c_str() ), m_line, column, tokenStr.length() ) );
            }
        }

        column += tokenLen + 1;
    }

    tokens.push_back( Token( l_eol, m_line, column, 0 ) );

    ++m_line;
}

} // namespace Carlo

} // namespace Agape
