#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/AssetLoader.h"
#include "Assets/ANSIFile.h"
#include "Assets/SAUCE.h"
#include "Clocks/Clock.h"
#include "InputDevices/InputDevice.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/LiteStream.h"
#include "World/Height.h"
#include "World/WorldCoordinates.h"
#include "ANSIEditor.h"
#include "ANSITerminal.h"
#include "ANSIWriter.h"
#include "String.h"
#include "Terminal.h"

#include <math.h>
#include <string.h>

#include "Loggers/Logger.h"

using namespace Agape::InputDevices;
using namespace Agape::World;

namespace
{
    const int defaultHeight( 10 );
    const int defaultWidth( 20 );

    const int cursorBlinkPeriod( 100 ); // ms
    
    const int set1Size( 10 );
    const int set2Size( 10 );
    const char set1[set1Size] = { '\x00', '\x08', '\x10', '\x18', '\xb0',
                                  '\xb8', '\xc0', '\xc8', '\xd0', '\xd8' };
    const char set2[set2Size] = { '\x80', '\x88', '\x90', '\x98', '\xa0',
                                  '\xa8', '\xe0', '\xe8', '\xf0', '\xf8' };
    
    const char set2Keys[8] = { 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i' };

    const int numForbiddenChars( 3 );
    const char forbiddenChars[numForbiddenChars] = { '\x0a', '\x0d', '\x1b' };

    // Cursor names for selection square.
    const char* _s1( "s1" );
    const char* _s2( "s2" );
    const char* _s3( "s3" );
    const char* _s4( "s4" );
} // Anonymous namespace

namespace Agape
{

namespace ANSIEditor
{

ANSIEditor::ANSIEditor( Terminal& terminal,
                        Terminal& toolboxTerminal,
                        Terminal& statusTerminal,
                        AssetLoaders::Factory& assetLoaderFactory,
                        const World::Coordinates& coordinates,
                        const String& assetName,
                        const String& author,
                        Timers::Factory& timerFactory,
                        Clock& clock ) :
  m_terminal( terminal ),
  m_toolboxTerminal( toolboxTerminal ),
  m_statusTerminal( statusTerminal ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_coordinates( coordinates ),
  m_assetName( assetName ),
  m_author( author ),
  m_timer( timerFactory.makeTimer() ),
  m_openMode( modeNone ),
  m_clock( clock ),
  m_currentAssetLoader( nullptr ),
  m_row( 0 ),
  m_col( 0 ),
  m_cursorAttributes( 0x07 ),
  m_frameX( -1 ),
  m_frameY( -1 ),
  m_height( 0 ),
  m_width( 0 ),
  m_blit( false ),
  m_sprite( false ),
  m_animate( false ),
  m_animFrames( 0 ),
  m_fgColour( 0x07 ),
  m_bgColour( 0 ),
  m_set1Idx( 4 ),
  m_set2Idx( 9 ),
  m_literal( false ),
  m_haveSelection( false ),
  m_selectionX( 0 ),
  m_selectionY( 0 ),
  m_selectionHeight( 0 ),
  m_selectionWidth( 0 ),
  m_clipHeight( 0 ),
  m_clipWidth( 0 ),
  m_heightMap( new char[terminal.height() * terminal.width()]),
  m_clipHeightMap( new char[terminal.height() * terminal.width()]),
  m_showHeights( false ),
  m_modified( false ),
  m_haveUndo( false ),
  m_undoBackspace( false ),
  m_undoRow( 0 ),
  m_undoCol( 0 ),
  m_undoCharacter( 0x00 ),
  m_undoAttribute( 0x00 )
{
    ::memset( m_heightMap, 0, terminal.height() * terminal.width() );
    ::memset( m_clipHeightMap, 0, terminal.height() * terminal.width() );
}

ANSIEditor::~ANSIEditor()
{
    delete( m_timer );
    delete( m_currentAssetLoader );
    delete[]( m_heightMap );
    delete[]( m_clipHeightMap );
}

bool ANSIEditor::open( int openMode )
{
    bool success( false );

    m_openMode = openMode;

    m_currentAssetLoader = m_assetLoaderFactory.makeLoader( m_coordinates, m_assetName );

    if( m_currentAssetLoader->open( AssetLoader::modeRead, String() ) )
    {
        Assets::ANSIFile ansiFile( *m_currentAssetLoader );

        m_height = ansiFile.height();
        m_width = ansiFile.width();

        defineFrame(); // Required for getHeights(), below.

        if( ansiFile.hasSAUCE() )
        {
            // Copy existing SAUCE and extract heights and flags
            m_sauce = ansiFile.getSAUCE();

            ansiFile.getHeights( m_heightMap,
                                m_terminal.height(),
                                m_terminal.width(),
                                m_frameY + 1,
                                m_frameX + 1 );

            ansiFile.getAssetFlags( m_blit,
                                    m_sprite,
                                    m_animate,
                                    m_animFrames,
                                    m_templateName );
        }
        else
        {
            setSAUCEDefaults();
        }

        success = true;
    }
    else
    {
        delete( m_currentAssetLoader );
        m_currentAssetLoader = nullptr;

        if( openMode & modeCreate )
        {
            m_height = defaultHeight;
            m_width = defaultWidth;

            setSAUCEDefaults();

            success = true;
        }
    }

    return success;
}

void ANSIEditor::draw()
{
    m_terminal.clearScreen();
    m_toolboxTerminal.clearScreen();
    m_statusTerminal.clearScreen();

    drawFrame();

    if( m_currentAssetLoader )
    {
        Assets::ANSIFile ansiFile( *m_currentAssetLoader );

        m_terminal.consumeNext( m_frameY + 1, m_frameX + 1 );
        m_terminal.consumeAsset( ansiFile,
                                 0,
                                 ansiFile.dataSize(),
                                 m_width,
                                 m_frameX + 1,
                                 Terminal::noMaxRow,
                                 Terminal::scrollLock,
                                 Terminal::ANSI );
    }

    m_terminal.consumeNext( m_frameY + 1, m_frameX + 1 );
    m_terminal.enableTerminalCursor( true );

    m_row = 0;
    m_col = 0;

    m_showHeights = false;
    m_terminal.moveCursor( "Terminal", m_frameY + 1, m_frameX + 1, '_' );

    if( m_openMode & modeWrite )
    {
        drawToolbox();
        drawCharacters();
    }
    drawFlags();
}

void ANSIEditor::redraw()
{
    if( m_showHeights )
    {
        // Redraw heights that have been blatted by an overdrawn
        // dialogue box, etc.
        int absStartRow( m_frameY + 1 );
        int absStartCol( m_frameX + 1 );
        int absCursorRow( absStartRow + m_row );
        int absCursorCol( absStartCol + m_col );
        m_terminal.setCursorVisible( "Terminal", false );
        drawAllHeights();
        int height( *( m_heightMap + ( absCursorRow * m_terminal.width() ) + absCursorCol ) );
        m_terminal.moveCursor( "Terminal", absCursorRow, absCursorCol, charFromHeight( height ) );
        m_terminal.setCursorVisible( "Terminal", true );
    }
}

void ANSIEditor::consumeCharacter( char c )
{
    // Switch height mode
    if( c == control( 'h' ) )
    {
        switchHeightMode();
    }
    // Move cursor
    else if( c == Key::up )
    {
        cancelSelection();
        tryMoveCursor( m_row - 1, m_col );
    }
    else if( c == Key::down )
    {
        cancelSelection();
        tryMoveCursor( m_row + 1, m_col );
    }
    else if( c == Key::left )
    {
        cancelSelection();
        tryMoveCursor( m_row, m_col - 1 );
    }
    else if( c == Key::right )
    {
        cancelSelection();
        tryMoveCursor( m_row, m_col + 1 );
    }
    else if( m_showHeights && ( m_openMode & modeWrite ) )
    {
        if( ( c == 'f' ) || ( c == 'b' ) || ( c == 'g' ) )
        {
            setHeight( c );
            m_modified = true;
        }
        // Lock out all other keys if showing height overlay
        return;
    }
    // Switch foreground or background colour
    else if( c == control( Key::up ) && ( m_openMode & modeWrite ) )
    {
        int newFG( m_fgColour - 1 );
        if( newFG < 0 ) newFG = 0x0F;
        updateToolbox( m_fgColour, newFG, true );
        m_fgColour = newFG;
        drawCharacters();
    }
    else if( c == control( Key::down ) && ( m_openMode & modeWrite ) )
    {
        int newFG( m_fgColour + 1 );
        if( newFG > 0x0F ) newFG = 0;
        updateToolbox( m_fgColour, newFG, true );
        m_fgColour = newFG;
        drawCharacters();
    }
    else if( c == control( Key::left ) && ( m_openMode & modeWrite ) )
    {
        int newBG( m_bgColour - 1 );
        if( newBG < 0 ) newBG = 0x0F;
        updateToolbox( m_bgColour, newBG, false );
        m_bgColour = newBG;
        drawCharacters();
    }
    else if( c == control( Key::right ) && ( m_openMode & modeWrite ) )
    {
        int newBG( m_bgColour + 1 );
        if( newBG > 0x0F ) newBG = 0;
        updateToolbox( m_bgColour, newBG, false );
        m_bgColour = newBG;
        drawCharacters();
    }
    // Drag selection rectangle
    else if( c == Key::shiftUp && ( m_openMode & modeWrite ) )
    {
        tryDragSelection( m_row - 1, m_col );
    }
    else if( c == Key::shiftDown && ( m_openMode & modeWrite ) )
    {
        tryDragSelection( m_row + 1, m_col );
    }
    else if( c == Key::shiftLeft && ( m_openMode & modeWrite ) )
    {
        tryDragSelection( m_row, m_col - 1 );
    }
    else if( c == Key::shiftRight && ( m_openMode & modeWrite ) )
    {
        tryDragSelection( m_row, m_col + 1 );
    }
    // Adjust image size
    else if( c == control( '/' ) && ( m_openMode & modeWrite ) )
    {
        tryResize( m_height - 1, m_width );
        drawFlags();
        m_modified = true;
    }
    else if( c == control( '\'' ) && ( m_openMode & modeWrite ) )
    {
        tryResize( m_height + 1, m_width );
        drawFlags();
        m_modified = true;
    }
    else if( c == control( ',' ) && ( m_openMode & modeWrite ) )
    {
        tryResize( m_height, m_width - 1 );
        drawFlags();
        m_modified = true;
    }
    else if( c == control( '.' ) && ( m_openMode & modeWrite ) )
    {
        tryResize( m_height, m_width + 1 );
        drawFlags();
        m_modified = true;
    }
    // Adjust animation frames
    else if( c == control( 'k' ) && ( m_openMode & modeWrite ) )
    {
        trySetAnimationFrames( m_animFrames - 1 );
        drawFlags();
        m_modified = true;
    }
    else if( c == control( 'l' ) && ( m_openMode & modeWrite ) )
    {
        trySetAnimationFrames( m_animFrames + 1 );
        drawFlags();
        m_modified = true;
    }
    // Set flags
    else if( c == control( 'b' ) && ( m_openMode & modeWrite ) )
    {
        m_blit = !m_blit;
        drawFlags();
        m_modified = true;
    }
    else if( c == control( 'r' ) && ( m_openMode & modeWrite ) )
    {
        m_sprite = !m_sprite;
        drawFlags();
        m_modified = true;
    }
    else if( c == control( 'j' ) && ( m_openMode & modeWrite ) )
    {
        m_templateName.erase();
        drawFlags();
        m_modified = true;
    }
    // Copy, cut, paste.
    else if( ( c == control( 'c' ) && ( m_openMode & modeWrite ) ) && m_haveSelection )
    {
        clipCopy();
        cancelSelection();
        m_modified = true;
    }
    else if( ( c == control( 'x' ) && ( m_openMode & modeWrite ) ) && m_haveSelection )
    {
        clipCut();
        cancelSelection();
        m_modified = true;
    }
    else if( c == control( 'v' ) && ( m_openMode & modeWrite ) )
    {
        cancelSelection();
        clipPaste();
        m_modified = true;
    }
    else if( c == control( 'z' ) && m_haveUndo )
    {
        cancelSelection();
        undo();
    }
    // Switch character keybinds
    else if( c == Key::tab && ( m_openMode & modeWrite ) )
    {
        if( !m_literal )
        {
            ++m_set1Idx;
            if( m_set1Idx == set1Size )
            {
                m_set1Idx = 0;
                m_literal = true;
            }
        }
        else
        {
            m_literal = false;
        }
        drawCharacters();
    }
    else if( ( c >= control( '0' ) ) && ( c <= control( '9' ) ) )
    {
        // Quick set set1
        m_set1Idx = c - control( '0' );
        m_set1Idx += 3;
        m_set1Idx %= 10;
        m_literal = false;
        drawCharacters();
    }
    else if( c == control( 'o' ) )
    {
        // Quick toggle literal
        m_literal = !m_literal;
        drawCharacters();
    }
    else if( c == Key::shiftTab && ( m_openMode & modeWrite ) )
    {
        ++m_set2Idx;
        if( m_set2Idx == ( set1Size + set2Size ) ) m_set2Idx = 0;
        drawCharacters();
    }
    // Backspace/newline/insert
    else if( c == Key::backspace && ( m_openMode & modeWrite ) )
    {
        cancelSelection();
        tryBackspace();
        m_modified = true;
    }
    else if( c == '\n' && ( m_openMode & modeWrite ) )
    {
        cancelSelection();
        tryMoveCursor( m_row + 1, 0 ); // Shift lines down if insert?
    }
    else if( c >= 0x20 && ( m_openMode & modeWrite ) ) // && c <= 0x7F, which is implied.
    {
        cancelSelection();
        tryType( c );
        m_modified = true;
    }
}

const String& ANSIEditor::assetName() const
{
    return m_assetName;
}

const String& ANSIEditor::getTemplateName() const
{
    return m_templateName;
}

void ANSIEditor::setTemplateName( const String& templateName )
{
    m_templateName = templateName;
    drawFlags();
    m_modified = true;
}

int ANSIEditor::fgColour() const
{
    return m_fgColour;
}

int ANSIEditor::bgColour() const
{
    return m_bgColour;
}

void ANSIEditor::setFGColour( int fgColour )
{
    m_fgColour = fgColour;
}

void ANSIEditor::setBGColour( int bgColour )
{
    m_bgColour = bgColour;
}

bool ANSIEditor::modified()
{
    return m_modified;
}

bool ANSIEditor::close( bool doSave )
{
    return( close( doSave, String() ) );
}

bool ANSIEditor::close( bool doSave, const String& as )
{
    bool success( true );

    if( ( m_openMode & modeWrite ) && doSave )
    {
        save( as );
    }

    if( success && m_currentAssetLoader )
    {
        success = m_currentAssetLoader->close();
        delete( m_currentAssetLoader );
        m_currentAssetLoader = nullptr;
    }

    if( success && m_haveSelection ) cancelSelection();

    if( success ) m_terminal.enableTerminalCursor( false );

    return success;
}

bool ANSIEditor::save()
{
    return save( String() );
}

bool ANSIEditor::save( const String& as )
{
    String saveName( m_assetName );
    if( !as.empty() ) saveName = as;

    ANSIWriter ansiWriter( m_assetLoaderFactory,
                            m_coordinates,
                            m_clock,
                            saveName );

    if( ansiWriter.write( m_terminal.buffer(),
                          m_heightMap,
                          m_terminal.height(),
                          m_terminal.width(),
                          m_frameY + 1,
                          m_frameX + 1,
                          m_height,
                          m_width,
                          m_sauce,
                          m_blit,
                          m_sprite,
                          m_animFrames,
                          m_templateName ) )
    {
        m_modified = false;
        return true;
    }

    return false;
}

void ANSIEditor::run()
{
    if( m_timer->ms() >= cursorBlinkPeriod )
    {
        if( m_cursorAttributes == 0x00 )
        {
            m_cursorAttributes = 0x07;
        }
        else
        {
            m_cursorAttributes = 0x00;
        }
        
        m_terminal.moveCursor( "Terminal", m_frameY + m_row + 1, m_frameX + m_col + 1, 0, m_cursorAttributes );

        m_timer->reset();
    }
}

void ANSIEditor::defineFrame()
{
    m_frameX = ::floor( ( m_terminal.width() - m_width ) / 2 ) - 1;
    m_frameY = ::floor( ( m_terminal.height() - m_height ) / 2 ) - 1;
}

void ANSIEditor::drawFrame( bool erase )
{
    defineFrame();

    char* heightPtr( nullptr );

    // Top line
    if( m_frameY >= 0 )
    {
        m_terminal.consumeNext( m_frameY, m_frameX + 1 );
        for( int i = 0; i < m_width; ++i )
        {
            char c = erase ? '\x00' : '\xc4';
            m_terminal.consumeChar( c, Terminal::scrollLock );
        }

        // Reset height map
        heightPtr = m_heightMap + ( m_frameY * m_terminal.width() ) + m_frameX + 1;
        ::memset( heightPtr, '\0', m_width );
    }

    // Bottom line
    if( ( m_frameY + m_height + 1 ) < ( m_terminal.height() ) )
    {
        m_terminal.consumeNext( m_frameY + m_height + 1, m_frameX + 1 );
        for( int i = 0; i < m_width; ++i )
        {
            char c = erase ? '\x00' : '\xc4';
            m_terminal.consumeChar( c, Terminal::scrollLock );
        }

        // Reset height map
        heightPtr = m_heightMap + ( ( m_frameY + m_height + 1 ) * m_terminal.width() ) + m_frameX + 1;
        ::memset( heightPtr, '\0', m_width );
    }

    // Sides and corners
    int animFrameHeight( 0 );
    bool animValid( false );
    bool drawingAnimFrame( false );

    if( m_animFrames > 0 )
    {
        animFrameHeight = m_height / m_animFrames;
        if( animFrameHeight == 0 ) animFrameHeight = 1;
    }

    if( ( animFrameHeight > 0 ) &&
        ( m_animFrames <= m_height ) &&
        ( ( m_height % m_animFrames ) == 0 ) )
    {
        animValid = true;
    }

    for( int y = m_frameY; y <= m_frameY + m_height + 1; ++y )
    {
        int imageRow( y - m_frameY - 1 );

        // Normal sides
        char c1 = erase ? '\x00' : '\xb3';
        char c2 = erase ? '\x00' : '\xb3';
        int attributes( Terminal::colGrey );

        if( m_animate )
        {
            if( ( ( imageRow % animFrameHeight ) == 0 ) &&
                ( imageRow < m_height ) )
            {
                // Frame top
                c1 = erase ? '\x00' : '\xda';
                c2 = erase ? '\x00' : '\xbf';
                drawingAnimFrame = true;
                attributes = animValid ? Terminal::colGreen : Terminal::colRed;
            }
            else if( ( ( ( imageRow + 1 ) % animFrameHeight ) == 0 ) &&
                     ( imageRow >= 0 ) &&
                     ( imageRow < m_height ) )
            {
                // Frame bottom
                c1 = erase ? '\x00' : '\xc0';
                c2 = erase ? '\x00' : '\xd9';
                drawingAnimFrame = false;
                attributes = animValid ? Terminal::colGreen : Terminal::colRed;
            }
            else if( drawingAnimFrame &&
                     ( imageRow < m_height ) )
            {
                // Frame sides
                attributes = animValid ? Terminal::colGreen : Terminal::colRed;
            }
        }

        if( y == m_frameY )
        {
            // Top corners
            c1 = erase ? '\x00' : '\xda';
            c2 = erase ? '\x00' : '\xbf';
        }
        else if( y == m_frameY + m_height + 1 )
        {
            // Bottom corners
            c1 = erase ? '\x00' : '\xc0';
            c2 = erase ? '\x00' : '\xd9';
        }

        // Ensure x and y coords for both sides within screen visible area
        if( ( y >= 0 ) && ( y < m_terminal.height() ) )
        {
            if( m_frameX >= 0 )
            {
                m_terminal.consumeNext( y, m_frameX, attributes );
                m_terminal.consumeChar( c1, Terminal::scrollLock );

                // Reset height map
                heightPtr = m_heightMap + ( y * m_terminal.width() ) + m_frameX;
                ::memset( heightPtr, '\0', 1 );
            }
            if( ( m_frameX + m_width + 1 ) < m_terminal.width() )
            {
                m_terminal.consumeNext( y, m_frameX + m_width + 1, attributes );
                m_terminal.consumeChar( c2, Terminal::scrollLock );

                // Reset height map
                heightPtr = m_heightMap + ( y * m_terminal.width() ) + m_frameX + m_width + 1;
                ::memset( heightPtr, '\0', 1 );
            }
        }
    }
}

void ANSIEditor::drawToolbox()
{
    m_toolboxTerminal.clearScreen();

    for( int ystep = 0; ystep < 8; ++ystep )
    {
        m_toolboxTerminal.consumeNext( ystep * 3, 0, ystep );
        m_toolboxTerminal.consumeString( "\xdb\xdb\xdb", Terminal::scrollLock );
        m_toolboxTerminal.consumeNext( ystep * 3, 5, ystep + 8 );
        m_toolboxTerminal.consumeString( "\xdb\xdb\xdb", Terminal::scrollLock );
        m_toolboxTerminal.consumeNext( ( ystep * 3 ) + 1, 0, ystep );
        m_toolboxTerminal.consumeString( "\xdb\xdb\xdb", Terminal::scrollLock );
        m_toolboxTerminal.consumeNext( ( ystep * 3 ) + 1, 5, ystep + 8 );
        m_toolboxTerminal.consumeString( "\xdb\xdb\xdb", Terminal::scrollLock );
    }

    drawToolboxMarker( m_fgColour, true, true );
    drawToolboxMarker( m_bgColour, false, true );
}

void ANSIEditor::updateToolbox( int oldColour, int newColour, bool fg )
{
    drawToolboxMarker( oldColour, fg, false );
    drawToolboxMarker( newColour, fg, true );
}

void ANSIEditor::drawToolboxMarker( int colour, bool fg, bool on )
{
    int row = ( colour % 8 ) * 3;
    int col = ( colour < 8 ) ? 0 : 5;
    if( fg )
    {
        row += 1;
        col += 2;
    }

    char c;
    int attribute;
    if( on )
    {
        if( fg )
        {
            c = 'f';
        }
        else
        {
            c = 'b';
        }
        attribute = colour << 4;
        if( colour < 8 ) attribute += 0x0F;
    }
    else
    {
        c = '\xdb';
        attribute = colour;
    }

    m_toolboxTerminal.consumeNext( row, col, attribute );
    m_toolboxTerminal.consumeChar( c, Terminal::scrollLock );
}

void ANSIEditor::drawCharacters()
{
    m_statusTerminal.consumeNext( 0, 0 );

    for( int i = 0; i < 8; ++i )
    {
        m_statusTerminal.setAttributes( 0x07 );
        if( i != 0 ) m_statusTerminal.consumeString( "  " );
        m_statusTerminal.consumeChar( '1' + i );
        m_statusTerminal.setAttributes( ( m_bgColour << 4 ) + m_fgColour );
        char c( set1[m_set1Idx] + i );
        if( !m_literal && !isForbiddenChar( c ) )
        {
            m_statusTerminal.consumeChar( c, Terminal::scrollLock, Terminal::literal );
        }
        else
        {
            m_statusTerminal.consumeChar( '1' + i, Terminal::scrollLock );
        }
    }

    m_statusTerminal.consumeString( "  " );

    for( int i = 0; i < 8; ++i )
    {
        m_statusTerminal.setAttributes( 0x07 );
        m_statusTerminal.consumeString( "  " );
        m_statusTerminal.consumeChar( set2Keys[i] );
        m_statusTerminal.setAttributes( ( m_bgColour << 4 ) + m_fgColour );
        char c( '\0' );
        if( m_set2Idx < set1Size )
        {
            c = set1[m_set2Idx] + i;
        }
        else
        {
            c = set2[m_set2Idx - set1Size] + i;
        }
        if( !m_literal && !isForbiddenChar( c ) )
        {
            m_statusTerminal.consumeChar( c, Terminal::scrollLock, Terminal::literal );
        }
        else
        {
            m_statusTerminal.consumeChar( set2Keys[i], Terminal::scrollLock );
        }
    }

    m_statusTerminal.setAttributes( 0x07 );
}

void ANSIEditor::drawFlags()
{
    m_statusTerminal.consumeNext( 0, 65 );

    LiteStream stream;
    if( m_blit )
    {
        stream << ANSITerminal::colours( Terminal::colGreen, Terminal::colWhite );
    }
    else
    {
        stream << ANSITerminal::colours( Terminal::colBlue, Terminal::colGrey );
    }
    stream << "B" << ANSITerminal::reset() << " ";

    if( m_sprite )
    {
        stream << ANSITerminal::colours( Terminal::colGreen, Terminal::colWhite );
    }
    else
    {
        stream << ANSITerminal::colours( Terminal::colBlue, Terminal::colGrey );
    }
    stream << "R" << ANSITerminal::reset() << " ";

    if( m_animate )
    {
        stream << ANSITerminal::colours( Terminal::colGreen, Terminal::colWhite )
               << "A";
        if( m_animFrames < 10 ) stream << " ";
        stream << m_animFrames;
    }
    else
    {
        stream << ANSITerminal::colours( Terminal::colBlue, Terminal::colGrey )
               << "A  ";
    }
    stream << ANSITerminal::reset() << " ";

    if( !m_templateName.empty() )
    {
        stream << ANSITerminal::colours( Terminal::colGreen, Terminal::colWhite );
    }
    else
    {
        stream << ANSITerminal::colours( Terminal::colBlue, Terminal::colGrey );
    }
    stream << "T" << ANSITerminal::reset() << " ";

    if( m_width < 10 ) stream << " ";
    stream << m_width;
    stream << "x" << m_height;
    if( m_height < 10 ) stream << " ";

    m_statusTerminal.consumeString( stream.str(), Terminal::scrollLock );
}

bool ANSIEditor::tryMoveCursor( int row, int col )
{
    if( ( row >= 0 ) && ( row < m_height ) &&
        ( col >= 0 ) && ( col < m_width ) )
    {
        if( !m_showHeights )
        {
            m_terminal.moveCursor( "Terminal", m_frameY + row + 1, m_frameX + col + 1, '_' );
        }
        else
        {
            // Draw height overlay at previous position.
            int prevAbsRow( m_frameY + m_row + 1 );
            int prevAbsCol( m_frameX + m_col + 1 );
            m_terminal.setCursorVisible( "Terminal", false );
            m_terminal.consumeNext( prevAbsRow, prevAbsCol );
            int prevHeight( *( m_heightMap + ( prevAbsRow * m_terminal.width() ) + prevAbsCol ) );
            m_terminal.consumeChar( charFromHeight( prevHeight ), Terminal::scrollLock, Terminal::transient | Terminal::blit );

            // Move cursor to new position.
            int absRow( m_frameY + row + 1 );
            int absCol( m_frameX + col + 1 );
            int height( *( m_heightMap + ( absRow * m_terminal.width() ) + absCol ) );
            m_terminal.moveCursor( "Terminal", absRow, absCol, charFromHeight( height ) );
            m_terminal.setCursorVisible( "Terminal", true );
        }

        m_row = row;
        m_col = col;
        
        return true;
    }

    return false;
}

void ANSIEditor::tryBackspace()
{
    if( m_col > 0 )
    {
        int absRow( m_frameY + m_row + 1 );
        int absCol( m_frameX + m_col + 1 );
        if( m_col == ( m_width - 1 ) )
        {
            // Cursor at end of line. Delete character at end of line if it's
            // not already NUL, else delete character second from end.
            char character( 0 );
            char attributes( 0 );
            m_terminal.getBaseCharAt( absRow, absCol, character, attributes );
            if( character == '\0' )
            {
                --m_col;
                --absCol;
            }
        }
        else
        {
            --m_col;
            --absCol;
        }

        m_terminal.getBaseCharAt( absRow, absCol, m_undoCharacter, m_undoAttribute );
        m_undoRow = absRow;
        m_undoCol = absCol;
        m_haveUndo = true;
        m_undoBackspace = true;

        m_terminal.consumeNext( absRow, absCol, 0x00 );
        m_terminal.consumeChar( '\x00', Terminal::scrollLock );
        m_terminal.moveCursor( "Terminal", absRow, absCol );
    }
}

void ANSIEditor::tryType( char c )
{
    if( !m_literal )
    {
        bool setKeyMatched( false );
        int setIdx( 0 );
        bool usingSet2( false );
        char setChar( '\0' );

        if( ( c >= '1' ) && ( c <= '8' ) )
        {
            setKeyMatched = true;
            setIdx = c - '1';
        }
        else
        {
            for( int i = 0; i < 8; ++i )
            {
                if( c == set2Keys[i] )
                {
                    setKeyMatched = true;
                    usingSet2 = true;
                    setIdx = i;
                    break;
                }
            }
        }

        if( setKeyMatched )
        {
            if( !usingSet2 )
            {
                setChar = set1[m_set1Idx] + setIdx;
            }
            else
            {
                if( m_set2Idx < set1Size )
                {
                    setChar = set1[m_set2Idx] + setIdx;
                }
                else
                {
                    setChar = set2[m_set2Idx - set1Size] + setIdx;
                }
            }

            if( !isForbiddenChar( setChar ) ) c = setChar;
        }
    }

    int absRow( 0 );
    int absCol( 0 );
    m_terminal.getCursor( "Terminal", absRow, absCol );
    m_terminal.getBaseCharAt( absRow, absCol, m_undoCharacter, m_undoAttribute );
    m_undoRow = absRow;
    m_undoCol = absCol;
    m_haveUndo = true;
    m_undoBackspace = false;

    m_terminal.setAttributes( ( m_bgColour << 4 ) + m_fgColour );
    m_terminal.consumeChar( c, Terminal::scrollLock, Terminal::ANSI );

    if( m_col < m_width - 1 )
    {
        ++m_col;
    }
}

bool ANSIEditor::isForbiddenChar( char c )
{
    for( int i = 0; i < numForbiddenChars; ++i )
    {
        if( forbiddenChars[i] == c ) return true;
    }

    return false;
}

void ANSIEditor::switchHeightMode()
{
    int absStartRow( m_frameY + 1 );
    int absStartCol( m_frameX + 1 );
    int absCursorRow( absStartRow + m_row );
    int absCursorCol( absStartCol + m_col );

    if( !m_showHeights )
    {
        m_terminal.setCursorVisible( "Terminal", false );
        drawAllHeights();
        int height( *( m_heightMap + ( absCursorRow * m_terminal.width() ) + absCursorCol ) );
        m_terminal.moveCursor( "Terminal", absCursorRow, absCursorCol, charFromHeight( height ) );
        m_terminal.setCursorVisible( "Terminal", true );

        m_showHeights = true;
    }
    else
    {
        m_terminal.moveCursor( "Terminal", absCursorRow, absCursorCol, '_' );
        m_terminal.repaint( absStartRow, absStartCol, m_height, m_width );
        
        m_showHeights = false;
    }
}

char ANSIEditor::charFromHeight( int height )
{
    char c( '\0' );

    if( height == foreground )
    {
        c = 'F';
    }
    else if( height == background )
    {
        c = 'B';
    }
    else if( height == ground )
    {
        c = 'G';
    }

    return c;
}

void ANSIEditor::drawAllHeights()
{
    char* heightMapRowPtr( m_heightMap + ( ( m_frameY + 1 ) * m_terminal.width() ) );
    int heightMapWidthBytes( m_terminal.width() );
    for( int y = m_frameY + 1; y < ( m_frameY + m_height + 1 ); ++y )
    {
        m_terminal.consumeNext( y, ( m_frameX + 1 ) );
        for( int x = m_frameX + 1; x < ( m_frameX + m_width + 1 ); ++x )
        {
            m_terminal.consumeChar( charFromHeight( *( heightMapRowPtr + x ) ), Terminal::scrollLock, Terminal::transient | Terminal::blit );
        }
        heightMapRowPtr += heightMapWidthBytes;
    }
}

void ANSIEditor::setHeight( char c )
{
    int absRow( m_frameY + m_row + 1 );
    int absCol( m_frameX + m_col + 1 );
    char* heightPtr( m_heightMap + ( absRow * m_terminal.width() ) + absCol );

    switch( c )
    {
    case 'b':
        *heightPtr = 0;
        break;
    case 'f':
        *heightPtr = 1;
        break;
    case 'g':
        *heightPtr = 2;
        break;
    default:
        break;
    }

    // This duplicates what tryMoveCursor would do, but is needed for the case
    // where the cursor is at the far right (and tryMoveCursor would not
    // redraw the current position).
    m_terminal.moveCursor( "Terminal", absRow, absCol, charFromHeight( *heightPtr ) );

    tryMoveCursor( m_row, m_col + 1 );
}

void ANSIEditor::tryDragSelection( int row, int col )
{
    int prevRow( m_row );
    int prevCol( m_col );

    if( tryMoveCursor( row, col ) )
    {
        if( m_haveSelection )
        {
            if( row > prevRow )
            {
                if( ( prevRow != m_selectionY ) || ( m_selectionHeight == 0 ) )
                {
                    ++m_selectionHeight;
                }
                else
                {
                    m_selectionY = row;
                    --m_selectionHeight;
                }
            }
            else if( row < prevRow )
            {
                if( prevRow != m_selectionY )
                {
                    --m_selectionHeight;
                }
                else
                {
                    m_selectionY = row;
                    ++m_selectionHeight;
                }
            }
            if( col > prevCol )
            {
                if( ( prevCol != m_selectionX ) || ( m_selectionWidth == 0 ) )
                {
                    ++m_selectionWidth;
                }
                else
                {
                    m_selectionX = col;
                    --m_selectionWidth;
                }
            }
            else if( col < prevCol )
            {
                if( prevCol != m_selectionX )
                {
                    --m_selectionWidth;
                }
                else
                {
                    m_selectionX = col;
                    ++m_selectionWidth;
                }
            }
        }
        else
        {
            m_selectionY = ( row < prevRow ) ? row : prevRow;
            m_selectionX = ( col < prevCol ) ? col : prevCol;
            m_selectionHeight = ( row != prevRow ) ? 1 : 0;
            m_selectionWidth = ( col != prevCol ) ? 1 : 0;
            m_haveSelection = true;
        }

        updateSelection();
    }
}

void ANSIEditor::updateSelection()
{
    int absX1( m_frameX + 1 + m_selectionX );
    int absX2( m_frameX + 1 + m_selectionX + m_selectionWidth );
    int absY1( m_frameY + 1 + m_selectionY );
    int absY2( m_frameY + 1 + m_selectionY + m_selectionHeight );

    if( m_selectionHeight == 0 )
    {
        cursorCreateOrMove( _s1, absY1, absX1, '\xb3', 0x0F );
        cursorCreateOrMove( _s2, absY1, absX2, '\xb3', 0x0F );
        m_terminal.setCursorVisible( _s3, false );
        m_terminal.setCursorVisible( _s4, false );
    }
    else if( m_selectionWidth == 0 )
    {
        cursorCreateOrMove( _s1, absY1, absX1, '\xc4', 0x0F );
        cursorCreateOrMove( _s2, absY2, absX1, '\xc4', 0x0F );
        m_terminal.setCursorVisible( _s3, false );
        m_terminal.setCursorVisible( _s4, false );
    }
    else
    {
        cursorCreateOrMove( _s1, absY1, absX1, '\xda', 0x0F );
        cursorCreateOrMove( _s2, absY1, absX2, '\xbf', 0x0F );
        cursorCreateOrMove( _s3, absY2, absX1, '\xc0', 0x0F );
        cursorCreateOrMove( _s4, absY2, absX2, '\xd9', 0x0F );
        m_terminal.setCursorVisible( _s3, true );
        m_terminal.setCursorVisible( _s4, true );
    }
}

void ANSIEditor::cursorCreateOrMove( const String& name, int row, int col, char glyph, char attributes )
{
    int currentRow( 0 );
    int currentCol( 0 );
    if( m_terminal.getCursor( name, currentRow, currentCol ) )
    {
        m_terminal.moveCursor( name, row, col, glyph );
    }
    else
    {
        m_terminal.createCursor( name, row, col, glyph, attributes );
    }
}

void ANSIEditor::cancelSelection()
{
    m_terminal.deleteCursor( _s1 );
    m_terminal.deleteCursor( _s2 );
    m_terminal.deleteCursor( _s3 );
    m_terminal.deleteCursor( _s4 );
    m_haveSelection = false;
}

void ANSIEditor::tryResize( int height, int width )
{
    if( ( height == 0 ) || ( height > m_terminal.height() ) ||
        ( width == 0 ) || ( width > m_terminal.width() ) )
    {
        return;
    }

    drawFrame( true ); // true = erase.

    m_height = height;
    m_width = width;

    drawFrame();

    int newRow( ( m_row >= m_height ) ? m_height - 1 : m_row );
    int newCol( ( m_col >= m_width ) ? m_width - 1 : m_col );
    tryMoveCursor( newRow, newCol );
}

void ANSIEditor::trySetAnimationFrames( int animFrames )
{
    if( animFrames > 0 )
    {
        m_animate = true;
        m_animFrames = animFrames;
    }
    else
    {
        m_animate = false;
        m_animFrames = 0;
    }

    drawFrame();
}

void ANSIEditor::clipCopy()
{
    // Copy selection to draw terminal.
    int absX( m_frameX + 1 + m_selectionX );
    int absY( m_frameY + 1 + m_selectionY );
    m_terminal.clipCopy( absY, absX, m_selectionHeight + 1, m_selectionWidth + 1 );

    m_clipHeight = m_selectionHeight + 1;
    m_clipWidth = m_selectionWidth + 1;

    // Copy selection height map to temporary height map
    clipCopyHeightMap( false );

    tryMoveCursor( m_selectionY, m_selectionX );
}

void ANSIEditor::clipCut()
{
    int absX( m_frameX + 1 + m_selectionX );
    int absY( m_frameY + 1 + m_selectionY );
    m_terminal.clipCut( absY, absX, m_selectionHeight + 1, m_selectionWidth + 1 );

    m_clipHeight = m_selectionHeight + 1;
    m_clipWidth = m_selectionWidth + 1;

    // Copy selection height map to temporary height map and reset cut region
    clipCopyHeightMap( true );

    tryMoveCursor( m_selectionY, m_selectionX );
}

void ANSIEditor::clipPaste()
{
    int absX( m_frameX + 1 + m_col );
    int absY( m_frameY + 1 + m_row );
    m_terminal.clipPaste( absY, absX, m_frameY + m_height + 1, m_frameX + m_width + 1 );

    // Paste from temporary height map into paste location
    clipPasteHeightMap();
}

void ANSIEditor::clipCopyHeightMap( bool doZero )
{
    int absX( m_frameX + 1 + m_selectionX );
    int absY( m_frameY + 1 + m_selectionY );

    int heightMapWidth( m_terminal.width() );

    int heightMapSourceStartOffset( ( absY * heightMapWidth ) + absX );
    int heightMapDestStartOffset( 0 );
    int copyWidth( m_selectionWidth );

    char* sourcePtr( m_heightMap + heightMapSourceStartOffset );
    char* destPtr( m_clipHeightMap + heightMapDestStartOffset );
    for( int copyRow = absY; copyRow < ( absY + m_clipHeight ); ++copyRow )
    {
        ::memcpy( destPtr, sourcePtr, copyWidth );
        if( doZero ) ::memset( sourcePtr, '\0', copyWidth ); // For cut.
        sourcePtr += heightMapWidth;
        destPtr += heightMapWidth;
    }
}

void ANSIEditor::clipPasteHeightMap()
{
    int absX( m_frameX + 1 + m_col );
    int absY( m_frameY + 1 + m_row );

    int heightMapWidth( m_terminal.width() );

    int heightMapSourceStartOffset( 0 );
    int heightMapDestStartOffset( ( absY * heightMapWidth ) + absX );
    int copyWidth( m_clipWidth );

    char* sourcePtr( m_clipHeightMap + heightMapSourceStartOffset );
    char* destPtr( m_heightMap + heightMapDestStartOffset );
    for( int copyRow = 0; copyRow < m_clipHeight; ++copyRow )
    {
        ::memcpy( destPtr, sourcePtr, copyWidth );
        sourcePtr += heightMapWidth;
        destPtr += heightMapWidth;
    }
}

void ANSIEditor::undo()
{
    m_terminal.consumeNext( m_undoRow, m_undoCol, m_undoAttribute );
    m_terminal.consumeChar( m_undoCharacter, Terminal::scrollLock, Terminal::ANSI );

    m_row = m_undoRow - m_frameY - 1;
    m_col = m_undoCol - m_frameX - 1;

    if( m_undoBackspace && ( m_col < m_width - 1 ) )
    {
        // Try to position cursor to character after one that was backspaced.
        ++m_col;
    }

    int absX( m_frameX + 1 + m_col );
    int absY( m_frameY + 1 + m_row );
    m_terminal.moveCursor( "Terminal", absY, absX );

    m_haveUndo = false;
}

void ANSIEditor::setSAUCEDefaults()
{
    m_sauce.setAuthor( m_author );
    m_sauce.setDateFromClock( m_clock );
    m_sauce.setDataType( SAUCE::dtCharacter );
    m_sauce.setFileType( SAUCE::ftcANSI );
    m_sauce.setTFlags( SAUCE::flgiCEColours | SAUCE::flgLetterSpacing8px | SAUCE::flgAspectRatioModern );
    // Note: ANSIWriter will set FileSize, TInfo1 and TInfo2 (width and
    // number of lines) on save.
}

} // namespace ANSIEditor

} // namespace Agape
