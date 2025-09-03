#include "Utils/LiteStream.h"
#include "CarloHighlighter.h"
#include "Collections.h"
#include "ExecutionContext.h"
#include "Lexer.h"
#include "Linda2.h"
#include "Parser.h"
#include "ProgramManager.h"
#include "String.h"
#include "Terminal.h"

using namespace Agape::Carlo;

namespace
{
    const int runtimeErrorBackground( Agape::Terminal::colRed );
} // Anonymous namespace

namespace Agape
{

namespace Editor
{

namespace Highlighters
{

Carlo::Carlo( TupleRouter& tupleRouter,
              FunctionDispatcher& functionDispatcher,
              Agape::Carlo::ProgramManager& programManager ) :
  m_tupleRouter( tupleRouter ),
  m_functionDispatcher( functionDispatcher ),
  m_programManager( programManager ),
  m_previousLogicalLine( -1 )
{
}

void Carlo::line( const String& line, int logicalLine )
{
    if( logicalLine == m_previousLogicalLine )
    {
        // Remove previous EOL if line wraps.
        m_lexerTokens.pop_back();
    }
    
    m_lexer.lex( line, m_lexerTokens );
    m_previousLogicalLine = logicalLine;
}

void Carlo::highlight( const String& instanceName,
                       bool modified,
                       Vector< Token >& highlights,
                       Vector< String >& parseErrors,
                       Vector< String >& runtimeErrors )
{
    Parser parser( m_lexerTokens,
                   m_tupleRouter, // Router and dispatcher not used, but must be passed.
                   m_functionDispatcher,
                   false ); // false = Don't build abstract parse tree.
    
    Agape::Carlo::Linda2* linda2( nullptr );
    Vector< Parser::Error > _parserErrors;
    parser.parse( linda2, &highlights, &_parserErrors );

    {
    Vector< Parser::Error >::const_iterator it( _parserErrors.begin() );
    for( ; it != _parserErrors.end(); ++it )
    {
        LiteStream stream;
        stream << it->m_line + 1 << "," << it->m_column + 1
               << ": Error " << it->m_errorCode << ": "
               << Parser::s_errorMessages[it->m_errorCode];
        if( it->m_errorCode != Parser::errDuplicateArgumentName )
        {
            stream << ", found " << Lexer::s_string[it->m_lexerCode] << ".";
        }
        parseErrors.push_back( stream.str() );
    }
    }

    if( !modified )
    {
        Vector< ExecutionContext::RuntimeError > _runtimeErrors( m_programManager.runtimeErrors( instanceName ) );
        Vector< ExecutionContext::RuntimeError >::const_iterator it( _runtimeErrors.begin() );
        for( ; it != _runtimeErrors.end(); ++it )
        {
            LiteStream stream;
            stream << it->m_line + 1 << "," << it->m_col + 1
                << ": Runtime error R" << it->m_code << ": "
                << ExecutionContext::s_errorMessages[it->m_code] << ".";
            runtimeErrors.push_back( stream.str() );

            Vector< Token >::iterator tokenIt( highlights.begin() );
            for( ; tokenIt != highlights.end(); ++tokenIt )
            {
                bool didMatch( false );
                // In line() we lexed on *screen* lines, not actual program
                // lines, so program line numbers (from runtime errors - it)
                // may be out of step with screen line highlights (tokenIt) if
                // any lines wrapped, therefore allow inexact line number match.
                if( ( tokenIt->m_line >= it->m_line ) && ( tokenIt->m_column == it->m_col ) && ( tokenIt->m_length == it->m_len ) )
                {
                    // Set error background colour.
                    tokenIt->m_attributes = Terminal::attributes( runtimeErrorBackground, tokenIt->m_attributes & 0x0F );
                    didMatch = true;
                }

                if( didMatch )
                {
                    break;
                }
            }
        }
    }
}

} // namespace Highlighters

} // namespace Editor

} // namespace Agape
