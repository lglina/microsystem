#include "ANSITerminal.h"
#include "GraphicsDrivers/GraphicsDriver.h"
#include "Timers/Timer.h"
#include "Utils/LiteStream.h"
#include "String.h"

#include <cstddef>
#include <cstdlib>

//#include <iostream>

namespace
{

const unsigned int ansiColourMap[16] =
{
    0,
    4,
    2,
    6,
    1,
    5,
    3,
    7,
    8,
    12,
    10,
    14,
    9,
    13,
    11,
    15
};

} // anonymous namespace

namespace Agape
{

ANSITerminal::ANSITerminal( int width,
                            int height,
                            const String& windowName,
                            GraphicsDriver& graphicsDriver,
                            Timer& timer,
                            Terminal* drawTerminal,
                            bool haveBuffer ) :
    Terminal( width, height, windowName, graphicsDriver, timer, drawTerminal, haveBuffer ),
    m_consumingSequence( false ),
    m_ansiForegroundColour( 7 ),
    m_ansiBackgroundColour( 0 ),
    m_sgrBold( false ),
    m_sgrBlink( false ),
    m_charset( 0 )
{
}

void ANSITerminal::consumeChar( char c,
                                bool scrollLock,
                                int charMode,
                                char* drawMap,
                                int mapValue,
                                char charset )
{
    consumeChar( c, m_width, 0, scrollLock, charMode, drawMap, mapValue, charset );
}

void ANSITerminal::consumeChar( char c,
                                int width,
                                int widthOffset,
                                bool scrollLock,
                                int charMode,
                                char* drawMap,
                                int mapValue,
                                char charset )
{
    if( m_consumingSequence )
    {
        m_currentSequence += c;
        if( c == 'm' )
        {
            m_consumingSequence = false;
            parseSGRSequence();
            m_currentSequence.clear();
        }
        else if( c == 'A' || c == 'B' || c == 'C' || c == 'D' )
        {
            m_consumingSequence = false;
            parseCursorSequence();
            m_currentSequence.clear();
        }
    }
    else
    {
        if( ( c == '\x1b' ) && !( charMode & literal ) )
        {
            m_consumingSequence = true;
            m_currentSequence += c;
        }
        else if( ( c == '\x08' ) && !( charMode & literal ) && !( charMode & ANSI ) )
        {
            if( m_col > 0 )
            {
                --m_col;

                eraseCharacter( charMode );

                if( m_terminalCursorEnabled )
                {
                    moveCursor( "Terminal", m_row, m_col );
                }
            }
        }
        else if( ( c == '\x0d' ) && !( charMode & literal ) )
        {
            m_col = widthOffset;
        }
        else if( ( c == '\x0a' ) && !( charMode & literal ) )
        {
            if( m_row < ( m_height - 1 ) || scrollLock )
            {
                ++m_row;
            }
            else
            {
                scrollScreen( Terminal::scrollDown );
            }
        }
        else
        {
            if( m_row >= 0 && m_row < m_height && m_col >= 0 && m_col < m_width ) // Drop character if scroll lock on and we're off the screen.
            {
                // Allow partial transprency for partial box drawing characters.
                // FIXME: This doesn't yet work properly - any redraw and we get
                // a black background again, as we don't store the underlying
                // character that has been blitted over.
                /*
                if( ( charMode & whitespaceTransparency ) &&
                    ( ( c >= '\xb3' ) && ( c <= '\xdf' ) ) &&
                    ( c != '\xdb' ) &&
                    ( ( m_characterAttributes & 0xF0 ) == 0 ) )
                {
                    charMode |= blit;
                    charMode |= transient; // FIXME: ARGHHH! Will make semiblocks draw properly, but they won't refresh properly.
                }
                */

                Character cx;
                cx.m_row = m_row;
                cx.m_col = m_col;
                cx.m_character = c;
                cx.m_attributes = m_characterAttributes;
                if( charset == -1 )
                {
                    cx.m_charset = m_charset; // Use current terminal charset
                }
                else
                {
                    cx.m_charset = charset; // Use caller explicitly specified
                }

                if( !isTransparentWhitespace( cx, charMode ) )
                {
                    putCharacter( cx, charMode );
                }

                if( ( drawMap != nullptr ) && !isTransparentWhitespace( cx, charMode ) )
                {
                    int offset( ( m_row * m_width ) + m_col );
                    drawMap[offset] = mapValue;
                }
            }

            ++m_col;
            if( m_col == widthOffset + width )
            {
                m_col = widthOffset;
                if( m_row < ( m_height - 1 ) || scrollLock )
                {
                    ++m_row;
                }
                else
                {
                    scrollScreen( Terminal::scrollDown );
                }
            }

            if( m_terminalCursorEnabled )
            {
                int cursorRow = m_row;
                int cursorCol = m_col;
                if( cursorRow < 0 ) cursorRow = 0;
                if( cursorRow >= m_height ) cursorRow = m_height - 1;
                if( cursorCol < 0 ) cursorCol = 0;
                if( cursorCol >= m_width ) cursorCol = m_width - 1;

                moveCursor( "Terminal", cursorRow, cursorCol );
            }
        }
    }
}

int ANSITerminal::countPrinting( const String& string ) const
{
    int numPrinting( 0 );
    bool consumingSequence( false );

    for( int i = 0; i < string.length(); ++i )
    {
        char c( string[i] );
        if( c == '\x1b' && !consumingSequence )
        {
            consumingSequence = true;
        }
        else if( consumingSequence &&
                 ( ( c == 'm' ) ||
                   ( c == 'A' ) ||
                   ( c == 'B' ) ||
                   ( c == 'C' ) ||
                   ( c == 'D' ) ) )
        {
            consumingSequence = false;
        }
        else if( !consumingSequence )
        {
            ++numPrinting;
        }
    }

    return numPrinting;
}

String ANSITerminal::colours( int bgColour, int fgColour )
{
    int bgColourCode( 0 );
    int fgColourCode( 0 );

    if( fgColour <= 7 )
    {
        fgColourCode = 30 + ansiColourMap[fgColour];
    }
    else
    {
        fgColourCode = 90 + ansiColourMap[fgColour] - 8;
    }

    if( bgColour <= 7 )
    {
        bgColourCode = 40 + ansiColourMap[bgColour];
    }
    else
    {
        bgColourCode = 100 + ansiColourMap[bgColour] - 8;
    }

    LiteStream stream;
    stream << "\x1b[" << bgColourCode << ";" << fgColourCode << "m";
    return stream.str();
}

String ANSITerminal::colour( int fgColour )
{
    int fgColourCode( 0 );

    if( fgColour <= 7 )
    {
        fgColourCode = 30 + ansiColourMap[fgColour];
    }
    else
    {
        fgColourCode = 90 + ansiColourMap[fgColour] - 8;
    }

    LiteStream stream;
    stream << "\x1b[" << fgColourCode << "m";
    return stream.str();
}

String ANSITerminal::reset()
{
    return "\x1b[0m";
}

String ANSITerminal::charset( int charset )
{
    LiteStream stream;
    stream << "\x1b[" << ( 10 + charset ) << "m";
    return stream.str();
}

String ANSITerminal::resetCharset()
{
    return "\x1b[10m";
}

int ANSITerminal::fromTermColour( int termColour )
{
    return ansiColourMap[termColour];
}

void ANSITerminal::parseSGRSequence()
{
    std::size_t from( m_currentSequence.find( '[' ) );
    ++from;

    std::size_t nextDelim = m_currentSequence.find( ';', from);
    while( nextDelim != String::npos )
    {
        String match( m_currentSequence.substr( from, nextDelim - from ) );
        parseSGRAttribute( ::atoi( match.c_str() ) );
        from = nextDelim + 1;
        nextDelim = m_currentSequence.find( ';', from);
    }

    if( from != m_currentSequence.length() )
    {
        String match( m_currentSequence.substr( from, m_currentSequence.length() - from - 1 ) );
        parseSGRAttribute( ::atoi( match.c_str() ) );  
    }
}

void ANSITerminal::parseCursorSequence()
{
    String distanceStr( m_currentSequence.substr( 2, m_currentSequence.length() - 3 ) );
    int distance( ::atoi( distanceStr.c_str() ) );
    String direction( m_currentSequence.substr( m_currentSequence.length() - 1, 1 ) );

    switch( direction[0] )
    {
    case 'A':
        m_row -= distance;
        if( m_row < 0 ) { m_row = 0; };
        break;
    case 'B':
        m_row += distance;
        if( m_row > ( m_height - 1 ) ) { m_row = ( m_height - 1 ); };
        break;
    case 'C':
        m_col += distance;
        if( m_col > ( m_width - 1 ) ) { m_col = ( m_width - 1 ); };
        break;
    case 'D':
        m_col -= distance;
        if( m_col < 0 ) { m_col = 0; };
        break;
    }
}

void ANSITerminal::parseSGRAttribute( int attribute )
{
    // Create bright foreground colour on "bold" attribute,
    // and bright background colour on "blink" attribute (iCE colours).
    if( ( m_sgrBold && attribute >= 30 && attribute <= 37 ) ||
        ( m_sgrBlink && attribute >= 40 && attribute <= 47 ) )
    {
        attribute += 60;
    }

    if( attribute == 0 )
    {
        // Grey on black.
        m_ansiForegroundColour = 7;
        m_ansiBackgroundColour = 0;
        m_characterAttributes = ( ansiColourMap[m_ansiBackgroundColour] << 4 ) + ansiColourMap[m_ansiForegroundColour];
        m_sgrBold = 0;
        m_sgrBlink = 0;
    }
    else if( attribute == 1 )
    {
        m_sgrBold = 1;
        m_ansiForegroundColour |= 0x08;
        m_characterAttributes = ( m_characterAttributes & 0xF0 ) + ansiColourMap[m_ansiForegroundColour];
    }
    else if( attribute == 5 )
    {
        m_sgrBlink = 1;
        m_ansiBackgroundColour |= 0x08;
        m_characterAttributes = ( m_characterAttributes & 0x0F ) + ( ansiColourMap[m_ansiBackgroundColour] << 4 );
    }
    else if( attribute >= 10 && attribute <= 19 )
    {
        m_charset = attribute - 10;
    }
    else if( attribute >= 30 && attribute <= 37 )
    {
        m_ansiForegroundColour = attribute - 30;
        m_characterAttributes = ( m_characterAttributes & 0xF0 ) + ansiColourMap[m_ansiForegroundColour];
    }
    else if( attribute >= 90 && attribute <= 97 )
    {
        m_ansiForegroundColour = attribute - 90 + 8;
        m_characterAttributes = ( m_characterAttributes & 0xF0 ) + ansiColourMap[m_ansiForegroundColour];
    }
    else if( attribute >= 40 && attribute <= 47 )
    {
        m_ansiBackgroundColour = attribute - 40;
        m_characterAttributes = ( m_characterAttributes & 0x0F ) + ( ansiColourMap[m_ansiBackgroundColour] << 4 );
    }
    else if( attribute >= 100 && attribute <= 107 )
    {
        m_ansiBackgroundColour = attribute - 100 + 8;
        m_characterAttributes = ( m_characterAttributes & 0x0F ) + ( ansiColourMap[m_ansiBackgroundColour] << 4 );
    }
}

} // namespace Agape
