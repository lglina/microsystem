#include "Allocator.h"
#include "Assets/Asset.h"
#include "Timers/Timer.h"
#include "Utils/Cartesian.h"
#include "Utils/Tokeniser.h"
#include "Collections.h"
#include "String.h"
#include "Terminal.h"
#include "GraphicsDrivers/GraphicsDriver.h"

#include "stdlib.h"

#include <string.h>

#include "Loggers/Logger.h"

namespace
{

const int defaultAttributes( 0x07 ); // Grey on black.
const char defaultCursorGlyph( '\x5F' );

const int animationStepTime( 500 ); // ms

} // Anonymous namespace

namespace Agape
{

const int Terminal::colBlack( 0 );
const int Terminal::colBlue( 1 );
const int Terminal::colGreen( 2 );
const int Terminal::colCyan( 3 );
const int Terminal::colRed( 4 );
const int Terminal::colMagenta( 5 );
const int Terminal::colBrown( 6 );
const int Terminal::colGrey( 7 );
const int Terminal::colDarkGrey( 8 );
const int Terminal::colBrightBlue( 9 );
const int Terminal::colBrightGreen( 10 );
const int Terminal::colBrightCyan( 11 );
const int Terminal::colBrightRed( 12 );
const int Terminal::colBrightMagenta( 13 );
const int Terminal::colYellow( 14 );
const int Terminal::colWhite( 15 );

const int Terminal::noMaxHeight( -1 );
const int Terminal::noMaxWidth( -1 );
const bool Terminal::hCentre( true );
const bool Terminal::noHCentre( false );
const bool Terminal::vCentre( true );
const bool Terminal::noVCentre( false );
const bool Terminal::scrollLock( true );
const bool Terminal::scrollUnlock( false );
const int Terminal::noMaxRow( -1 );
const int Terminal::atCurrentRow( -1 );
const int Terminal::atCurrentCol( -1 );
const bool Terminal::scrollUp( false );
const bool Terminal::scrollDown( true );

const char Terminal::avatarCharset( 1 );

// This could be more safely/easily done with smart pointers, but IIRC they
// require exceptions, which we're avoiding using as they're too much of a
// memory hog in embedded environments.
Terminal::SpriteBuffer::SpriteBuffer( int height, int width, int numFrames ) :
  m_users( 1 )
{
    m_buffer = new char[height * width * numFrames * 2];
    m_map = new char[height * width];

    ::memset( m_map, '\0', height * width );
    // No need to memset buffer here, as we will overwrite it entirely
    // when copy into it from the sprite draw buffer.
}

Terminal::SpriteBuffer::~SpriteBuffer()
{
    delete[]( m_buffer );
    delete[]( m_map );
}

Map< String, Terminal::SpriteBuffer* > Terminal::Sprite::m_spriteBuffers;

Terminal::Sprite::Sprite( const String& name,
                          const String& assetName,
                          char row,
                          char col,
                          char height,
                          char width,
                          int charMode,
                          char numFrames ) :
  m_name( name ),
  m_assetName( assetName ),
  m_row( row ),
  m_col( col ),
  m_height( height ),
  m_width( width ),
  m_charMode( charMode ),
  m_numFrames( numFrames ),
  m_currentFrame( 0 ),
  m_newBuffer( true ),
  m_buffer( nullptr ),
  m_map( nullptr )
{
    Map< String, SpriteBuffer* >::iterator it( m_spriteBuffers.find( assetName ) );
    if( it == m_spriteBuffers.end() )
    {
        SpriteBuffer* spriteBuffer( new SpriteBuffer( height,
                                                      width,
                                                      numFrames ) );
        m_buffer = spriteBuffer->m_buffer;
        m_map = spriteBuffer->m_map;
        m_spriteBuffers.emplace( assetName, spriteBuffer );
    }
    else
    {
        SpriteBuffer* spriteBuffer( it->second );
        m_buffer = spriteBuffer->m_buffer;
        m_map = spriteBuffer->m_map;
        ++( spriteBuffer->m_users);
    }
}
        
Terminal::Sprite::~Sprite()
{
    Map< String, SpriteBuffer* >::iterator it( m_spriteBuffers.find( m_assetName ) );
    if( it != m_spriteBuffers.end() )
    {
        SpriteBuffer* spriteBuffer( it->second );
        if( --( spriteBuffer->m_users ) == 0 )
        {
            delete( spriteBuffer );
            m_spriteBuffers.erase( it );
        }
    } // else something has gone horribly wrong!
}

Terminal::Terminal( int width,
                    int height,
                    const String& windowName,
                    GraphicsDriver& graphicsDriver,
                    Timer& timer,
                    Terminal* drawTerminal,
                    bool haveBuffer ) :
  m_width( width ),
  m_height( height ),
  m_row( 0 ),
  m_col( 0 ),
  m_characterAttributes( defaultAttributes ),
  m_windowName( windowName ),
  m_terminalCursorEnabled( false ),
  m_graphicsDriver( graphicsDriver ),
  m_timer( timer ),
  m_drawTerminal( drawTerminal ),
  m_haveBuffer( haveBuffer ),
  m_buffer( nullptr ),
  m_clipHeight( 0 ),
  m_clipWidth( 0 ),
  m_cursorVariant( 0 )
{
    if( m_haveBuffer )
    {
        m_buffer = new char[width * height * 2];
        for( int i = 0; i < width*height; ++i )
        {
            *( m_buffer + ( i * 2 ) ) = 0; // rand();
            *( m_buffer + ( ( i * 2 ) + 1 ) ) = 0; // rand();
        }

        //repaint();
    }
}

Terminal::~Terminal()
{
    delete[]( m_buffer );

    deleteAllSprites( false );
}

int Terminal::width() const
{
    return m_width;
}

int Terminal::height() const
{
    return m_height;
}

int Terminal::row() const
{
    return m_row;
}

int Terminal::col() const
{
    return m_col;
}

void Terminal::repaint()
{
    repaint( 0, 0, m_height, m_width );
}

void Terminal::repaint( int row, int col, int height, int width )
{
    if( !m_haveBuffer ) return;

    // Draw static characters.
    for( int curRow = row; ( curRow < ( row + height ) ) && ( curRow < m_height ); ++curRow )
    {
        int offset( ( ( curRow * m_width ) + col ) * 2 );
        char* glyphsAttrs( m_buffer + offset );
        for( int curCol = col; ( curCol < ( col + width ) ) && ( curCol < m_width ); ++curCol )
        {
            m_graphicsDriver.paintGlyph( m_windowName, curRow, curCol, glyphsAttrs );
            glyphsAttrs += 2;
        }
    }

    // Redraw any parts of any sprites overlapping redraw area.
    Rectangle redrawRect( col,
                          row,
                          height,
                          width );
    Vector< Sprite* >::const_iterator spriteIt( m_sprites.begin() );
    for( ; spriteIt != m_sprites.end(); ++spriteIt )
    {
        const Sprite& sprite( **spriteIt );
        Rectangle spriteRect( sprite.m_col,
                              sprite.m_row,
                              sprite.m_height,
                              sprite.m_width );
        if( spriteRect.intersects( redrawRect ) )
        {
#ifdef LOG_WINDOWS
            LOG_DEBUG( "Sprite " + sprite.m_name + " with rect " + spriteRect.dump() + " intersects redraw region " + redrawRect.dump() );
#endif
            Rectangle intersection( spriteRect.findIntersection( redrawRect ) );
#ifdef LOG_WINDOWS
            LOG_DEBUG( "Intersection rect: " + intersection.dump() );
#endif
            drawSprite( sprite,
                        intersection.originY(),
                        intersection.originX(),
                        intersection.height(),
                        intersection.width() );
        }
    }

    // Draw cursors.
    Vector< Cursor >::const_iterator cursorIt;
    for( cursorIt = m_cursors.begin(); cursorIt != m_cursors.end(); ++cursorIt )
    {
        if( cursorIt->m_visible &&
            ( cursorIt->m_row >= row ) &&
            ( cursorIt->m_row < ( row + height ) ) &&
            ( cursorIt->m_col >= col ) &&
            ( cursorIt->m_col < ( col + width ) ) )
        {
            drawCursor( *cursorIt );
        }
    }
}

void Terminal::clearScreen( bool resetAttributes )
{
    m_graphicsDriver.clearScreen( m_windowName );
    m_row = 0;
    m_col = 0;
    if( m_haveBuffer ) ::memset( m_buffer, 0, m_height * m_width * 2 );

    deleteAllSprites( false );

    if( resetAttributes )
    {
        m_characterAttributes = defaultAttributes;
    }
}

void Terminal::clearLines( int from, int len, bool resetAttributes )
{
    if( ( from >= 0 ) && ( from < m_height ) &&
        ( len >= 0 ) && ( ( from + len ) <= m_height ) )
    {
        m_graphicsDriver.clearLines( m_windowName, from, len );
        m_row = from;
        m_col = 0;
        if( m_haveBuffer ) ::memset( m_buffer + ( m_width * from * 2 ), 0, m_width * len * 2 );

        // FIXME: Clear sprites overlapping? Haven't done this yet as this is only
        // used where sprites aren't used or relevant (e.g. chat window).

        if( resetAttributes )
        {
            m_characterAttributes = defaultAttributes;
        }
    }
}

void Terminal::fillScreen( char c, char attributes, int from, int len, int charMode )
{
    char* glyphsAttrs( m_buffer );
    glyphsAttrs += from * m_width * 2;

    for( m_row = from; m_row < ( len == -1 ? m_height : from + len ); ++m_row )
    {
        for( m_col = 0; m_col < m_width; ++m_col )
        {
            if( m_haveBuffer && !( charMode & transient ) )
            {
                *glyphsAttrs = c;
                *( glyphsAttrs + 1 ) = attributes;
                m_graphicsDriver.paintGlyph( m_windowName, m_row, m_col, glyphsAttrs );
                glyphsAttrs += 2;
            }
            else
            {
                char transientGlyphAttr[2] = { c, attributes };
                m_graphicsDriver.paintGlyph( m_windowName, m_row, m_col, transientGlyphAttr );
            }
        }
    }

    m_graphicsDriver.dumpPerformanceInfo();

    if( !( charMode & transient ) )
    {
        deleteAllSprites( false );
    }

    m_row = 0;
    m_col = 0;
}

void Terminal::scrollScreen( bool down, int from, int len )
{
    if( len == -1 ) len = m_height;

    // FIXME: Use hardware scroll, if supported by graphics driver.
    if( !m_haveBuffer ) return;

    //std::cerr << "Scroll screen" << std::endl;
    char* base( m_buffer );
    if( down )
    {
        for( int i = m_width * ( from + 1 ) * 2; i < m_width * ( from + len ) * 2; ++i )
        {
            *( base + i - ( m_width * 1 * 2 ) ) = *( base + i );
        }

        for( int i = m_width * ( from + len - 1 ) * 2; i < m_width * ( from + len ) * 2; ++i )
        {
            *( base + i ) = '\0';
        }
    }
    else
    {
        for( int i = ( m_width * ( from + len - 1 ) * 2 ) - 1; i >= ( m_width * from * 2 ); --i )
        {
            *( base + i + ( m_width * 2 ) ) = *( base + i );
        }

        for( int i = ( m_width * from * 2 ); i < m_width * ( from + 1 ) * 2; ++i )
        {
            *( base + i ) = '\0';
        }
    }
    repaint();
    
    // FIXME: Hardware scroll?
    // FIXME: Scroll sprite characters?
}

void Terminal::transientBlank()
{
    m_graphicsDriver.clearAll();
}

void Terminal::setAttributes( char attributes )
{
    m_characterAttributes = attributes;
}

void Terminal::consumeNext( int row, int col, char attributes )
{
    m_row = row;
    m_col = col;
    m_characterAttributes = attributes;

    if( m_row >= height() )
    {
        scrollScreen( Terminal::scrollDown );
        m_row = height() - 1;
    }

    if( m_terminalCursorEnabled )
    {
        moveCursor( "Terminal", m_row, m_col );
    }
}

void Terminal::consumeChar( char c,
                            bool scrollLock,
                            int charMode,
                            char* drawMap,
                            int mapValue,
                            char charset )
{
    // FIXME: Stub. Should just add chars verbatim to screen and scroll.
}

void Terminal::consumeChar( char c,
                            int width,
                            int widthOffset,
                            bool scrollLock,
                            int charMode,
                            char* drawMap,
                            int mapValue,
                            char charset )
{
    // FIXME: Stub. Should just add chars verbatim to screen and scroll.
}

void Terminal::consumeString( const String& string,
                              bool scrollLock,
                              int charMode,
                              char* drawMap,
                              int mapValue )
{
    for( String::size_type i = 0; i < string.length(); ++i )
    {
        consumeChar( string[i], m_width, 0, scrollLock, charMode, drawMap, mapValue );
    }
}

void Terminal::insertAtCursor( char c,
                               const String& cursorName,
                               int fieldStart,
                               int fieldWidth )
{
    if( fieldWidth == -1 ) fieldWidth = m_width;

    if( ( fieldStart < 0 ) ||
        ( fieldStart >= m_width ) ||
        ( ( fieldStart + fieldWidth ) > m_width ) )
    {
        return;
    }

    int row( -1 );
    int col( -1 );
    if( !getCursor( cursorName, row, col ) ) return;
    
    if( col < ( fieldStart + fieldWidth ) )
    {
        // Shuffle all characters and attributes along one position to end of
        // line. Character at end of line, if any, will be overwritten.
        int startOffset( ( ( m_width * 2 ) * row ) + ( col * 2 ) );
        int endOffset( ( ( m_width * 2 ) * row ) + ( ( fieldStart + fieldWidth ) * 2 ) - 2 );
        for( int i = endOffset; i >= startOffset; --i )
        {
            m_buffer[i+2] = m_buffer[i];
        }

        // Insert new. Preserve background attributes.
        m_buffer[startOffset] = c;
        m_buffer[startOffset + 1] = ( m_buffer[startOffset + 1] & 0xF0 ) + m_characterAttributes;

        if( cursorName == "Terminal" )
        {
            if( col < ( fieldStart + fieldWidth - 1 ) )
            {
                moveCursor( "Terminal", row, ++col );
            }
        }

        repaint( row, col, 1, m_width - col );
    }
}

void Terminal::backspaceAtCursor( const String& cursorName,
                                  int fieldStart,
                                  int fieldWidth )
{
    if( fieldWidth == -1 ) fieldWidth = m_width;

    if( ( fieldStart < 0 ) ||
        ( fieldStart >= m_width ) ||
        ( ( fieldStart + fieldWidth ) > m_width ) )
    {
        return;
    }

    int row( -1 );
    int col( -1 );
    if( !getCursor( cursorName, row, col ) ) return;
    
    if( col > fieldStart )
    {
        // Shuffle all characters and attributes back one position from end
        // of line.
        int startOffset( ( ( m_width * 2 ) * row ) + ( col * 2 ) );
        int endOffset( ( ( m_width * 2 ) * row ) + ( ( fieldStart + fieldWidth ) * 2 ) );
        for( int i = startOffset; i <= endOffset; ++i )
        {
            m_buffer[i-2] = m_buffer[i];
        }

        // Remove trailing character at end of line.
        m_buffer[endOffset - 2] = '\0';

        if( cursorName == "Terminal" )
        {
            moveCursor( "Terminal", row, --col );
        }

        repaint( row, col - 1, 1, m_width - col + 1 );
    }
}

void Terminal::insertAtCursor( char c )
{
    insertAtCursor( c, "Terminal" );
}

void Terminal::backspaceAtCursor()
{
    backspaceAtCursor( "Terminal" );
}

void Terminal::consumeAsset( const Asset& asset,
                             int len,
                             bool scrollLock,
                             int charMode,
                             char* drawMap,
                             int mapValue )
{
    consumeAsset( asset, 0, len, m_width, 0, -1, scrollLock, charMode, drawMap, mapValue );
}

int Terminal::consumeAsset( const Asset& asset,
                            int assetOffset,
                            int len,
                            int width,
                            int widthOffset,
                            int maxRow,
                            bool scrollLock,
                            int charMode,
                            char* drawMap,
                            int mapValue )
{
    const int bufSize( 256 );
    char buffer[ bufSize ];
    int lenRead( 0 );
    while( lenRead < len && ( ( maxRow == -1 ) || ( m_row < maxRow ) ) )
    {
        int lenRemain( len - lenRead );
        int lenToRead;
        if( lenRemain > bufSize )
        {
            lenToRead = bufSize;
        }
        else
        {
            lenToRead = lenRemain;
        }
        
        int thisLenRead( asset.read( buffer, assetOffset + lenRead, lenToRead ) );
        int i( 0 );
        for( ; ( i < thisLenRead ) && ( ( maxRow == -1 ) || ( m_row < maxRow ) ) ; ++i )
        {
            consumeChar( buffer[i], width, widthOffset, scrollLock, charMode, drawMap, mapValue );
        }

        lenRead += i;
    }

    return lenRead;
}

void Terminal::createSprite( const String& name,
                             const String& assetName,
                             const Asset& asset,
                             int len,
                             int row,
                             int col,
                             int height,
                             int width,
                             int charMode,
                             int numFrames )
{
    if( !m_haveBuffer || !m_drawTerminal ) return;

    Sprite* sprite = new Sprite( name,
                                 assetName,
                                 row,
                                 col,
                                 height,
                                 width,
                                 charMode,
                                 numFrames );

    int frameSize( height * width * 2 );
    int srcRowSize( m_drawTerminal->width() * 2 );
    int destRowSize( width * 2 );
    int assetOffset( 0 );

    for( int frameNum = 0; frameNum < numFrames; ++frameNum )
    {
        m_drawTerminal->clearScreen();
        assetOffset += m_drawTerminal->consumeAsset( asset,
                                                    assetOffset,
                                                    len - assetOffset,
                                                    width,
                                                    0,
                                                    height,
                                                    scrollLock,
                                                    charMode,
                                                    nullptr,
                                                    0 );

        int destStartOffset( frameNum * frameSize );

        for( int y = 0; y < height; ++y )
        {
            ::memcpy( sprite->m_buffer + destStartOffset + ( y * destRowSize ),
                      m_drawTerminal->m_buffer + ( y * srcRowSize ),
                      destRowSize );
        }
    }

    drawSprite( *sprite );

    m_sprites.push_back( sprite );
}

bool Terminal::isSprite( const String& name ) const
{
    Vector< Sprite* >::const_iterator it( m_sprites.begin() );
    for( ; it != m_sprites.end(); ++it )
    {
        if( ( *it )->m_name == name )
        {
            return true;
        }
    }

    return false;
}

bool Terminal::spriteData( const String& name,
                           String& assetName,
                           int& row,
                           int& col,
                           int& height,
                           int& width )
{
    Vector< Sprite* >::const_iterator it( m_sprites.begin() );
    for( ; it != m_sprites.end(); ++it )
    {
        const Sprite& sprite( **it );
        if( sprite.m_name == name )
        {
            assetName = sprite.m_assetName;
            row = sprite.m_row;
            col = sprite.m_col;
            height = sprite.m_height;
            width = sprite.m_width;
            return true;
        }
    }

    return false;
}

bool Terminal::spriteMap( const String& name,
                          char*& spriteMap )
{
    Vector< Sprite* >::const_iterator it( m_sprites.begin() );
    for( ; it != m_sprites.end(); ++it )
    {
        const Sprite& sprite( **it );
        if( sprite.m_name == name )
        {
            spriteMap = sprite.m_map;
            return true;
        }
    }

    return false;
}

void Terminal::moveSprite( const String& name,
                           int row,
                           int col )
{
    Vector< Sprite* >::iterator it( m_sprites.begin() );
    for( ; it != m_sprites.end(); ++it )
    {
        Sprite& sprite( **it );
        if( sprite.m_name == name )
        {
            int prevRow( sprite.m_row );
            int prevCol( sprite.m_col );

            sprite.m_row = row;
            sprite.m_col = col;

            // This will generate redundant draws if the new and old
            // positions overlap...
            repaint( prevRow, prevCol, sprite.m_height, sprite.m_width );
            repaint( sprite.m_row, sprite.m_col, sprite.m_height, sprite.m_width );

            break;
        }
    }
}

void Terminal::deleteSprite( const String& name )
{
    Vector< Sprite* >::iterator it( m_sprites.begin() );
    for( ; it != m_sprites.end(); ++it )
    {
        Sprite& sprite( **it );
        if( sprite.m_name == name )
        {
            int prevRow( sprite.m_row );
            int prevCol( sprite.m_col );
            int height( sprite.m_height );
            int width( sprite.m_width );
            delete( *it );
            m_sprites.erase( it );

            repaint( prevRow, prevCol, height, width );

            break;
        }
    }
}

void Terminal::deleteAllSprites( bool doRepaint )
{
    Vector< Sprite* >::iterator it( m_sprites.begin() );
    while( it != m_sprites.end() )
    {
        Sprite& sprite( **it );
        int prevRow( sprite.m_row );
        int prevCol( sprite.m_col );
        int height( sprite.m_height );
        int width( sprite.m_width );
        delete( *it );
        m_sprites.erase( it );
        
        if( doRepaint ) repaint( prevRow, prevCol, height, width );

        it = m_sprites.begin();
    }

    m_sprites.clear();
}

void Terminal::collideSprite( int row,
                              int col,
                              int height,
                              int width,
                              Vector< String >& names ) const
{
    Rectangle rectangle( col,
                         row,
                         height,
                         width );

    Vector< Sprite* >::const_iterator it( m_sprites.begin() );
    for( ; it != m_sprites.end(); ++it )
    {
        Sprite& sprite( **it );
        Rectangle spriteRect( sprite.m_col,
                              sprite.m_row,
                              sprite.m_height,
                              sprite.m_width );
        if( spriteRect.intersects( rectangle ) )
        {
            names.push_back( sprite.m_name );
        }
    }
}

void Terminal::consumeGraphicalAsset( const Asset& asset, int len )
{
    int assetOffset( 0 );

    int width;
    int height;
    asset.read( (char*)&width, assetOffset, 4 ); assetOffset += 4;
    asset.read( (char*)&height, assetOffset, 4 ); assetOffset += 4;

    char* lineBuffer = new char[ width * 3 ];
    int lineNum( 0 );
    while( lineNum < height )
    {
        // FIXME: Assumes no read errors.
        asset.read( lineBuffer, assetOffset, width * 3 );
        assetOffset += width * 3;

        m_graphicsDriver.paintBitmap( m_windowName, m_row, m_col, lineNum, 1, width, lineBuffer );

        ++lineNum;
    }

    delete[]( lineBuffer );
}

void Terminal::getBaseCharAt( int row, int col, char& character, char& attributes ) const
{
    if( !m_haveBuffer ) return;

    // FIXME: Return sprite characters here? Haven't done this yet as it's
    // only relevant for the map and it's probably not a big deal if sprites
    // don't show up on the map.
    int offset( ( ( row * m_width ) + col ) * 2 );
    character = *( m_buffer + offset );
    attributes = *( m_buffer + offset + 1 );
}

int Terminal::countPrinting( const String& string ) const
{
    return string.length();
}

int Terminal::printFormatted( const String& string )
{
    return printFormatted( string,
                           atCurrentRow,
                           atCurrentCol,
                           noMaxHeight,
                           noMaxWidth,
                           noHCentre,
                           noVCentre,
                           0x07,
                           preserveBackground );
}

int Terminal::printFormatted( const String& string,
                              int row,
                              int col,
                              int maxHeight,
                              int maxWidth,
                              bool hcentre,
                              bool vcentre,
                              char attributes,
                              int charmode )
{
    if( row == atCurrentRow )
    {
        row = m_row;
    }
    if( col == atCurrentCol )
    {
        col = m_col;
    }
    
    Tokeniser wordTokeniser( string, ' ' );
    Vector< String > lines;
    Vector< int > lineLengths;
    String line;
    int lineLength( 0 );
    int rowOffset( 0 );
    String currentWord( wordTokeniser.token() );

    if( maxWidth == noMaxWidth )
    {
        maxWidth = width() - col;
    }

    while( ( maxHeight == noMaxHeight ) || ( rowOffset < maxHeight ) )
    {
        if( !currentWord.empty() )
        {
            int wordLength( countPrinting( currentWord ) );
            int requiredLength( line.empty() ? wordLength : wordLength + 1 );
            if( ( lineLength + requiredLength ) <= maxWidth )
            {
                if( line == "" )
                {
                    line = currentWord;
                    lineLength = requiredLength;
                }
                else
                {
                    line += " " + currentWord;
                    lineLength += requiredLength;
                }
            }
            else
            {
                lines.push_back( line );
                lineLengths.push_back( lineLength );

                line = currentWord;
                lineLength = wordLength;
                ++rowOffset;
                // Word will overflow if word length > maxWidth.
            }
        }

        if( wordTokeniser.atEnd() )
        {
            break;
        }

        currentWord = wordTokeniser.token();
    }

    if( ( line != "" ) &&
        ( ( maxHeight == noMaxHeight ) || ( rowOffset < maxHeight ) ) )
    {
        lines.push_back( line );
        lineLengths.push_back( lineLength );
    }

    int firstRow( row );
    if( ( maxHeight != noMaxHeight ) && vcentre )
    {
        firstRow = ( row + ( maxHeight / 2 ) ) - ( lines.size() / 2 );
    }

    rowOffset = 0;
    Vector< String >::const_iterator linesIt( lines.begin() );
    Vector< int >::const_iterator lineLengthsIt( lineLengths.begin() );
    while( ( linesIt != lines.end() ) && ( lineLengthsIt != lineLengths.end() ) )
    {
        int firstCol( col );
        if( hcentre )
        {
            firstCol = ( col + ( maxWidth / 2 ) ) - ( *lineLengthsIt / 2 );
        }
        if( firstCol < col )
        {
            firstCol = col;
        }

        consumeNext( firstRow + rowOffset, firstCol, attributes );
        consumeString( *linesIt, true, charmode );
        ++rowOffset;
        ++linesIt;
        ++lineLengthsIt;
    }

    return lines.size();
}

void Terminal::createCursor( const String& name, int row, int col, char glyph, char attributes, char charset )
{
    Vector< Cursor >::iterator it;
    for( it = m_cursors.begin(); it != m_cursors.end(); ++it )
    {
        if( it->m_name == name )
        {
            return; // Don't duplicate
        }
    }

    Cursor cursor;
    cursor.m_name = name;
    cursor.m_row = row;
    cursor.m_col = col;
    cursor.m_glyph = glyph;
    cursor.m_charset = charset;
    cursor.m_attributes = attributes;
    cursor.m_visible = true;
    m_cursors.push_back( cursor );

    drawCursor( cursor );
}

void Terminal::deleteCursor( const String& name )
{
    Vector< Cursor >::iterator it;
    for( it = m_cursors.begin(); it != m_cursors.end(); ++it )
    {
        if( it->m_name == name )
        {
            int row( it->m_row );
            int col( it->m_col );

            m_cursors.erase( it );

            // Redraw characters and other cursors at this pos, if any.
            repaint( row, col, 1, 1 );

            break;
        }
    }
}

void Terminal::deleteAllCursors()
{
    Vector< Cursor >::iterator it;
    for( it = m_cursors.begin(); it != m_cursors.end(); ++it )
    {
        int row( it->m_row );
        int col( it->m_col );

        // Redraw characters and other cursors at this pos, if any.
        repaint( row, col, 1, 1 );
    }

    m_cursors.clear();
}

void Terminal::moveCursor( const String& name, int row, int col, char newGlyph, char newAttributes )
{
    Vector< Cursor >::iterator it;
    for( it = m_cursors.begin(); it != m_cursors.end(); ++it )
    {
        if( it->m_name == name )
        {
            int previousRow( it->m_row );
            int previousCol( it->m_col );

            it->m_row = row;
            it->m_col = col;

            if( name == "Terminal" )
            {
                // Move terminal insert/draw position.
                m_row = row;
                m_col = col;
            }

            if( newGlyph )
            {
                it->m_glyph = newGlyph;
            }
            
            if( newAttributes != -1 )
            {
                it->m_attributes = newAttributes;
            }

            if( it->m_visible )
            {
                // Redraw characters and other cursors at the previous pos, if any.
                repaint( previousRow, previousCol, 1, 1 );

                // Draw cursor at new pos.
                drawCursor( *it );
            }

            break;
        }
    }
}

bool Terminal::getCursor( const String& name, int& row, int& col ) const
{
    Vector< Cursor >::const_iterator it;
    for( it = m_cursors.begin(); it != m_cursors.end(); ++it )
    {
        if( it->m_name == name )
        {
            row = it->m_row;
            col = it->m_col;
            return true;
        }
    }

    return false;
}

void Terminal::setCursorVisible( const String& name, bool visible )
{
    Vector< Cursor >::iterator it;
    for( it = m_cursors.begin(); it != m_cursors.end(); ++it )
    {
        if( it->m_name == name )
        {
            if( visible && !it->m_visible )
            {
                it->m_visible = visible;
                drawCursor( *it );
            }
            else if( !visible && it->m_visible )
            {
                it->m_visible = visible;
                repaint( it->m_row, it->m_col, 1, 1 );
            }
            break;
        }
    }
}

void Terminal::setCursorsVisible( bool visible )
{
    Vector< Cursor >::iterator it;
    for( it = m_cursors.begin(); it != m_cursors.end(); ++it )
    {
        it->m_visible = visible;
    }
}

void Terminal::redrawCursors()
{
    Vector< Cursor >::const_iterator it;
    for( it = m_cursors.begin(); it != m_cursors.end(); ++it )
    {
        if( it->m_visible )
        {
            drawCursor( *it );
        }
    }
}

void Terminal::setCursorVariant( int variant )
{
    m_cursorVariant = variant;
}

void Terminal::enableTerminalCursor( bool enabled )
{
    if( enabled && !m_terminalCursorEnabled )
    {
        createCursor( "Terminal", m_row, m_col, defaultCursorGlyph, defaultAttributes );
    }
    else if( !enabled && m_terminalCursorEnabled )
    {
        deleteCursor( "Terminal" );
    }
    m_terminalCursorEnabled = enabled;
}

void Terminal::clipCopy( int row, int col, int height, int width )
{
    if( !m_drawTerminal ) return;

    if( ( row < 0 ) || ( ( row + height ) > m_height ) || ( ( row + height ) > m_drawTerminal->m_height ) ||
        ( col < 0 ) || ( ( col + width ) > m_width ) || ( ( col + width ) > m_drawTerminal->m_width ) ) return;

    int sourceRowWidthBytes( m_width * 2 );
    int destRowWidthBytes( m_drawTerminal->m_width * 2 );

    int sourceStartOffset( ( row * sourceRowWidthBytes ) + ( col * 2 ) );
    int copyWidthBytes( width * 2 );

    char* sourcePtr( m_buffer + sourceStartOffset );
    char* destPtr( m_drawTerminal->m_buffer );
    for( int copyRow = row; copyRow < ( row + height ); ++copyRow )
    {
        ::memcpy( destPtr, sourcePtr, copyWidthBytes );
        sourcePtr += sourceRowWidthBytes;
        destPtr += destRowWidthBytes;
    }

    m_clipHeight = height;
    m_clipWidth = width;
}

void Terminal::clipCut( int row, int col, int height, int width )
{
    if( !m_drawTerminal ) return;

    if( ( row < 0 ) || ( ( row + height ) > m_height ) || ( ( row + height ) > m_drawTerminal->m_height ) ||
        ( col < 0 ) || ( ( col + width ) > m_width ) || ( ( col + width ) > m_drawTerminal->m_width ) ) return;

    clipCopy( row, col, height, width );

    int rowWidthBytes( m_width * 2 );
    int startOffset( ( row * rowWidthBytes ) + ( col * 2 ) );
    int eraseWidthBytes( width * 2 );
    char* destPtr( m_buffer + startOffset );
    for( int eraseRow = row; eraseRow < ( row + height ); ++eraseRow )
    {
        ::memset( destPtr, '\0', eraseWidthBytes );
        destPtr += rowWidthBytes;
    }

    repaint( row, col, height, width );
}

void Terminal::clipPaste( int row, int col, int cropRow, int cropCol )
{
    if( !m_drawTerminal ) return;

    if( ( cropRow == -1 ) || ( cropRow > m_height ) ) cropRow = m_height;
    if( ( cropCol == -1 ) || ( cropCol > m_width ) ) cropCol = m_width;

    int sourceRowWidthBytes( m_drawTerminal->m_width * 2 );
    int destRowWidthBytes( m_width * 2 );

    int destStartOffset( ( row * destRowWidthBytes ) + ( col * 2 ) );
    int destMaxWidth( cropCol - col );
    int copyWidthBytes = ( m_clipWidth > destMaxWidth ) ? ( destMaxWidth * 2 ) : ( m_clipWidth * 2 );

    int destMaxHeight( cropRow - row );
    int copyHeight = ( m_clipHeight > destMaxHeight ) ? destMaxHeight : m_clipHeight;

    char* sourcePtr( m_drawTerminal->m_buffer );
    char* destPtr( m_buffer + destStartOffset );
    for( int copyRow = row; copyRow < ( row + copyHeight ); ++copyRow )
    {
        ::memcpy( destPtr, sourcePtr, copyWidthBytes );
        sourcePtr += sourceRowWidthBytes;
        destPtr += destRowWidthBytes;
    }

    repaint( row, col, m_clipHeight, m_clipWidth );
}

char Terminal::attributes( const String& colour )
{
    // No need to worry about overflow on return value,
    // as only returning a foreground attribute.
    if( colour == "black" )
    {
        return colBlack;
    }
    else if( colour == "blue" )
    {
        return colBlue;
    }
    else if( colour == "green" )
    {
        return colGreen;
    }
    else if( colour == "cyan" )
    {
        return colCyan;
    }
    else if( colour == "red" )
    {
        return colRed;
    }
    else if( colour == "magenta" )
    {
        return colMagenta;
    }
    else if( colour == "brown" )
    {
        return colBrown;
    }
    else if( colour == "grey" )
    {
        return colGrey;
    }
    else if( colour == "darkgrey" )
    {
        return colDarkGrey;
    }
    else if( colour == "bblue" )
    {
        return colBrightBlue;
    }
    else if( colour == "bgreen" )
    {
        return colBrightGreen;
    }
    else if( colour == "bcyan" )
    {
        return colBrightCyan;
    }
    else if( colour == "bred" )
    {
        return colBrightRed;
    }
    else if( colour == "bmagenta" )
    {
        return colBrightMagenta;
    }
    else if( colour == "yellow" )
    {
        return colYellow;
    }
    else if( colour == "white" )
    {
        return colWhite;
    }

    return colBlack;
}

char Terminal::attributes( int bgColour, int fgColour )
{
    unsigned char uc( fgColour + ( bgColour << 4 ) );
    return( *(char*)( &uc ) );
}

const char* Terminal::buffer() const
{
    return m_buffer;
}

void Terminal::flush()
{
    m_graphicsDriver.flush();
}

void Terminal::run()
{
    if( m_graphicsDriver.requestRedraw() )
    {
        repaint();
        m_graphicsDriver.redrawComplete();
    }

    if( m_timer.ms() >= animationStepTime )
    {
        m_timer.reset();

        // Step to next frame for all animated sprites. Unless a particular
        // character in the current frame is solid (i.e. not empty, not
        // transparent whitespace and not blitted) we need to redraw the
        // base character. The "proper" way to do this would be to completely
        // re-composite each character position to get the underlying character
        // plus any overlapping sprites plus blitting, but that seems
        // complicated and time-consuming. This is quick and dirty, with the
        // trade-off that animated sprites over each other, or animated over
        // non-animated, will not composite correctly.
        Vector< Sprite* >::iterator spriteIt( m_sprites.begin() );
        for( ; spriteIt != m_sprites.end(); ++spriteIt )
        {
            Sprite& sprite( **spriteIt );
            if( sprite.m_numFrames > 1 )
            {
                ++sprite.m_currentFrame;
                if( sprite.m_currentFrame >= sprite.m_numFrames )
                {
                    sprite.m_currentFrame = 0;
                }

                // Draw static characters.
                for( int rowOffset = 0; rowOffset < sprite.m_height; ++rowOffset )
                {
                    int spriteOffset( ( rowOffset * sprite.m_width ) * 2 );
                    int offset( ( ( ( sprite.m_row + rowOffset ) * m_width ) + sprite.m_col ) * 2 );

                    char* spriteGlyphsAttrs( sprite.m_buffer + spriteOffset );
                    char* glyphsAttrs( m_buffer + offset );

                    for( int colOffset = 0; colOffset < sprite.m_width; ++colOffset )
                    {
                        if( ( ( sprite.m_charMode & blit ) != 0 ) ||
                            ( *spriteGlyphsAttrs == '\x00' ) ||
                            ( ( ( sprite.m_charMode & whitespaceTransparency ) != 0 ) &&
                              isTransparentWhitespace( *spriteGlyphsAttrs,
                                                       *( spriteGlyphsAttrs + 1 ),
                                                       sprite.m_charMode ) ) )
                        {
                            m_graphicsDriver.paintGlyph( m_windowName,
                                                         sprite.m_row + rowOffset,
                                                         sprite.m_col + colOffset,
                                                         glyphsAttrs );
                            spriteGlyphsAttrs += 2;
                            glyphsAttrs += 2;
                        }
                    }
                }

                // Re-draw sprite.
                drawSprite( sprite );
            }
        }

        redrawCursors(); // In case animation frames have overwritten them.
    }
}

void Terminal::putCharacter( Character c, int charMode )
{
    int offset( ( ( c.m_row * m_width ) + c.m_col ) * 2 );
    char* charPtr( m_buffer + offset );
    char* attributePtr( m_buffer + offset + 1 );

    if( m_haveBuffer &&
        ( *charPtr == c.m_character ) &&
        ( *attributePtr == c.m_attributes ) &&
        ( ( charMode & ~whitespaceTransparency ) == 0 ) ) // whitespaceTransparency is don't care. All other bits (including force) must be zero.
    {
        // Don't bother to redraw if unchanged.
        // FIXME: Allow base character to be drawn over sprite? We'd need to
        // check for intersection with sprites here and allow draw
        // if intersecting.
        return;
    }

    unsigned char effectiveAttributes = c.m_attributes;
    if( m_haveBuffer && ( charMode & preserveBackground ) && ( ( c.m_attributes >> 4 ) == 0 ) )
    {
        // Use existing background if preserveBackground and
        // background of character to be drawn not set (zero/black).
        effectiveAttributes = (*attributePtr & 0xF0) + (c.m_attributes & 0x0F);
    }

    if( m_haveBuffer && !( charMode & transient ) ) // Non-transient character
    {
        // Note: We respect charset here, but don't have any way to save the
        // per-character charset in the terminal buffer. We could modify sprite
        // to allow these to be drawn in sprites. Similarly, we respect blitting
        // here, but blitting should only be done in sprites (or things won't
        // redraw properly).
        
        *charPtr = c.m_character;
        *attributePtr = effectiveAttributes;
        m_graphicsDriver.paintGlyph( m_windowName, c.m_row, c.m_col, charPtr, ( charMode & blit ), c.m_charset );
    }
    else
    {
        char glyphAttr[2] = { c.m_character, c.m_attributes };
        m_graphicsDriver.paintGlyph( m_windowName, c.m_row, c.m_col, glyphAttr, ( charMode & blit ), c.m_charset );
    }

    // We don't re-draw cursors here - the caller is expected to do this
    // (otherwise we'll slow things down by repeatedly redrawing cursors).
}

void Terminal::eraseCharacter( int charMode )
{
    if( !m_haveBuffer ) return;

    int offset( ( ( m_row * m_width ) + m_col ) * 2 );
    *( m_buffer + offset ) = '\0';
    if( !( charMode & preserveBackground ) )
    {
        *( m_buffer + offset + 1 ) = 0;
    }
    m_graphicsDriver.paintGlyph( m_windowName, m_row, m_col, m_buffer + offset );

    // This is probably only relevant for text fields, so cursors and/or sprite
    // characters are not erased here.
}

bool Terminal::isTransparentWhitespace( Character c, int charMode )
{
    return( isTransparentWhitespace( c.m_character, c.m_attributes, charMode ) );
}

bool Terminal::isTransparentWhitespace( char character, char attributes, int charMode )
{
    return( ( charMode & whitespaceTransparency ) && ( character == '\x00' || character == ' ' ) && ( ( *((unsigned char*)&attributes) & 0xF0 ) == 0 ) );
}

void Terminal::drawCursor( const Cursor& cursor )
{
    Character cx;
    cx.m_row = cursor.m_row;
    cx.m_col = cursor.m_col;
    cx.m_character = cursor.m_glyph;
    cx.m_attributes = cursor.m_attributes;
    cx.m_charset = cursor.m_charset;

    // Use charset 1 for avatars.
    int glyphidx( *( (unsigned char*)( &cursor.m_glyph ) ) );
    cx.m_charset = ( ( glyphidx >= 128 ) && ( glyphidx <= 163 ) ) ? 1 : 0;
    
    putCharacter( cx, blit | transient );

    if( m_cursorVariant == 1 ) // Christmas!
    {
        if( ( glyphidx >= 128 ) && ( glyphidx <= 163 ) )
        {
            if( ( glyphidx >= 128 ) && ( glyphidx <= 154 ) )
            {
                cx.m_character = '\xa4';
                cx.m_attributes = Terminal::colRed;
                putCharacter( cx, blit | transient );
                cx.m_character = '\xa5';
                cx.m_attributes = Terminal::colWhite;
                putCharacter( cx, blit | transient );
            }
            else
            {
                cx.m_character = '\xa6';
                cx.m_attributes = Terminal::colRed;
                putCharacter( cx, blit | transient );
                cx.m_character = '\xa7';
                cx.m_attributes = Terminal::colWhite;
                putCharacter( cx, blit | transient );
            }
        }
    }
}

void Terminal::drawSprite( const Sprite& sprite, int startRow, int startCol, int drawHeight, int drawWidth )
{
    // N.B. startRow and startCol are relative to origin of TERMINAL,
    // not sprite local origin.
    if( startRow == -1 ) startRow = sprite.m_row; // Draw from start.
    if( startCol == -1 ) startCol = sprite.m_col;
    if( drawHeight == -1 ) drawHeight = sprite.m_height; // Draw all.
    if( drawWidth == -1 ) drawWidth = sprite.m_width;

    int frameSize( sprite.m_height * sprite.m_width * 2 );
    int frameStartOffset( frameSize * sprite.m_currentFrame );

    int spriteRowSize( sprite.m_width * 2 );

    for( int y = 0; ( y < drawHeight ) && ( ( startRow + y ) < m_height ); ++y )
    {
        int spriteStartOffset( frameStartOffset +
                               ( ( startRow - sprite.m_row + y ) * spriteRowSize ) +
                               ( ( startCol - sprite.m_col ) * 2 ) );
        char* spriteCharPtr( sprite.m_buffer + spriteStartOffset );
        char* spriteAttributePtr( spriteCharPtr + 1 );

        for( int x = 0; ( x < drawWidth ) && ( ( startCol + x ) < m_width ); ++x )
        {
            if( !isTransparentWhitespace( *spriteCharPtr, *spriteAttributePtr, sprite.m_charMode ) )
            {
                char glyphAttr[2] = { *spriteCharPtr, *spriteAttributePtr };

                m_graphicsDriver.paintGlyph( m_windowName,
                                             startRow + y,
                                             startCol + x,
                                             glyphAttr,
                                             ( sprite.m_charMode & blit ),
                                             0 );
            }

            spriteCharPtr += 2;
            spriteAttributePtr += 2;
        }
    }
}

} // namespace Agape
