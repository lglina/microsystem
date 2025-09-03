#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/AssetLoader.h"
#include "Assets/ANSIFile.h"
#include "InputDevices/InputDevice.h"
#include "UI/Dialogue.h"
#include "UI/Hotkeys.h"
#include "UI/Navigation.h"
#include "Utils/LiteStream.h"
#include "World/WorldCoordinates.h"
#include "MiniMapStrategy.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"

#include <math.h>

using namespace Agape::InputDevices;
using namespace Agape::World;

using Agape::Terminal;

namespace
{
    const int attributesSelected( Agape::Terminal::colYellow );
    const int attributesNotSelected( Agape::Terminal::colGrey );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

MiniMap::MiniMap( AssetLoaders::Factory& assetLoaderFactory,
                  Coordinates& coordinates,
                  Navigation& navigation,
                  InputDevice& inputDevice,
                  Hotkeys& hotkeys,
                  WindowManager& windowManager,
                  const String& windowName,
                  Dialogue& dialogue ) :
  m_assetLoaderFactory( assetLoaderFactory ),
  m_coordinates( coordinates ),
  m_navigation( navigation ),
  m_inputDevice( inputDevice ),
  m_hotkeys( hotkeys ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_dialogue( dialogue ),
  m_completed( false ),
  m_terminal( nullptr ),
  m_xtiles( 3 ),
  m_ytiles( 3 ),
  m_xwidth( 0 ),
  m_yheight( 0 ),
  m_xborder( 0 ),
  m_yborder( 0 ),
  m_cursorx( 1 ),
  m_cursory( 1 )
{
}

MiniMap::~MiniMap()
{
}

void MiniMap::enter( const Value& parameters )
{
    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
    {
        m_completed = false;
        m_returnParameters = Value();
        m_terminal = terminalWindow.m_terminal;
        m_cursorx = 1;
        m_cursory = 1;
        m_centreCoordinates = m_coordinates;
        purgeCache();
        drawMaps();
        drawGrid( m_cursorx, m_cursory, true );
        m_navigation.draw( m_centreCoordinates );
        drawHotkeys();
    }
    else
    {
        m_completed = true; // Uh oh!
    }
}

void MiniMap::returnTo( const Value& parameters )
{
}

bool MiniMap::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool MiniMap::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        if( m_terminal )
        {
            m_terminal->repaint();
        }
        m_navigation.draw( m_coordinates ); // Draw current coords again.
        m_hotkeys.clear();
        parameters = m_returnParameters;
        return true;
    }

    return false;
}

void MiniMap::run()
{
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }

        if( c == Key::up )
        {
            drawGrid( m_cursorx, m_cursory, false );
            if( m_cursory > 0 )
            {
                --m_cursory;
            }
            else
            {
                m_centreCoordinates.m_y++;
                drawMaps();
            }
            drawGrid( m_cursorx, m_cursory, true );
            m_navigation.draw( coordsAtOffset( m_cursorx, m_cursory ) );
        }
        else if( c == Key::down )
        {
            drawGrid( m_cursorx, m_cursory, false );
            if( m_cursory < ( m_ytiles - 1 ) )
            {
                ++m_cursory;
            }
            else
            {
                m_centreCoordinates.m_y--;
                drawMaps();
            }
            drawGrid( m_cursorx, m_cursory, true );
            m_navigation.draw( coordsAtOffset( m_cursorx, m_cursory ) );
        }
        else if( c == Key::left )
        {
            drawGrid( m_cursorx, m_cursory, false );
            if( m_cursorx > 0 )
            {
                --m_cursorx;
            }
            else
            {
                m_centreCoordinates.m_x--;
                drawMaps();
            }
            drawGrid( m_cursorx, m_cursory, true );
            m_navigation.draw( coordsAtOffset( m_cursorx, m_cursory ) );
        }
        else if( c == Key::right )
        {
            drawGrid( m_cursorx, m_cursory, false );
            if( m_cursorx < ( m_xtiles - 1 ) )
            {
                ++m_cursorx;
            }
            else
            {
                m_centreCoordinates.m_x++;
                drawMaps();
            }
            drawGrid( m_cursorx, m_cursory, true );
            m_navigation.draw( coordsAtOffset( m_cursorx, m_cursory ) );
        }
        else if( ( c == '=' ) || ( c == '+' ) )
        {
            if( ( m_xtiles >= 5 ) && ( m_ytiles >= 5 ) )
            {
                m_xtiles -= 2;
                m_ytiles -= 2;
                m_cursorx = ::floor( m_xtiles / 2 );
                m_cursory = ::floor( m_ytiles / 2 );
                drawMaps();
                drawGrid( m_cursorx, m_cursory, true );
                m_navigation.draw( coordsAtOffset( m_cursorx, m_cursory ) );
            }
        }
        else if( ( c == '-' ) || ( c == '_' ) )
        {
            if( ( m_xtiles <= 7 ) && ( m_ytiles <= 7 ) )
            {
                m_xtiles += 2;
                m_ytiles += 2;
                m_cursorx = ::floor( m_xtiles / 2 );
                m_cursory = ::floor( m_ytiles / 2 );
                drawMaps();
                drawGrid( m_cursorx, m_cursory, true );
                m_navigation.draw( coordsAtOffset( m_cursorx, m_cursory ) );
            }
        }
        else if( c == Key::newLine )
        {
            m_returnParameters[_doTeleport] = 1;
            World::Coordinates cursorCoordinates( coordsAtOffset( m_cursorx, m_cursory ) );
            cursorCoordinates.toValue( m_returnParameters[_coordinates] );
            m_completed = true;
        }
        else if( c == Key::escape )
        {
            m_completed = true;
        }
    }
}

void MiniMap::purgeCache()
{
    if( m_cacheCoordinates.m_worldID != m_coordinates.m_worldID )
    {
        m_dialogue.show( Dialogue::normal );
        m_dialogue.drawTitle( "Please wait" );
        m_dialogue.drawMessage( "Resetting tile cache" );

        AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, String() ) );
        assetLoader->invalidateCached( true ); // true = invalidate all.
        delete( assetLoader );

        m_dialogue.hide();

        m_cacheCoordinates = m_coordinates;
    }
}

void MiniMap::drawMaps()
{
    m_xwidth = ::floor( m_terminal->width() / m_xtiles );
    m_yheight = ::floor( m_terminal->height() / m_ytiles );

    m_xborder = ::floor( ( m_terminal->width() - ( m_xwidth * m_xtiles ) ) / 2 );
    m_yborder = ::floor( ( m_terminal->height() - ( m_yheight * m_ytiles ) ) / 2 );

    // Black out borders.
    for( int y = m_yborder; y < ( m_yborder + ( m_ytiles * m_yheight ) ); ++y )
    {
        for( int x = 0; x < m_xborder; ++x )
        {
            m_terminal->consumeNext( y, x, 0 );
            m_terminal->consumeChar( '\xb0', Terminal::scrollLock, Terminal::transient );
        }

        for( int x = m_xborder + ( m_xtiles * m_xwidth ); x < m_terminal->width(); ++x )
        {
            m_terminal->consumeNext( y, x, 0 );
            m_terminal->consumeChar( '\xb0', Terminal::scrollLock, Terminal::transient );
        }
    }

    for( int y = 0; y < m_yborder; ++y )
    {
        for( int x = 0; x < m_terminal->width(); ++x )
        {
            m_terminal->consumeNext( y, x, 0 );
            m_terminal->consumeChar( '\xb0', Terminal::scrollLock, Terminal::transient );
        }
    }

    for( int y = m_yborder + ( m_ytiles * m_yheight ); y < m_terminal->height(); ++y )
    {
        for( int x = 0; x < m_terminal->width(); ++x )
        {
            m_terminal->consumeNext( y, x, 0 );
            m_terminal->consumeChar( '\xb0', Terminal::scrollLock, Terminal::transient );
        }
    }

    // Draw tiles.
    for( int x = 0; x < m_xtiles; ++x )
    {
        for( int y = 0; y < m_ytiles; ++y )
        {
            World::Coordinates coordinates( coordsAtOffset( x, y ) );

            // Create an asset name for each tile, based on the tile X and Y
            // coords and the number of screen tiles (as this affects the
            // tile size). The MiniMapAssetLoader will extract the number of
            // screen tiles from the asset name in order to render a tile of the
            // correct size, in the case where tiles need to be newly generated.
            // In the case where tiles are being cached, this unique name will
            // mean the tile for the right coords and for the right size can be
            // properly located.
            LiteStream stream;
            stream << coordinates.m_x << "_"
                   << coordinates.m_y << "T"
                   << m_xtiles << "_"
                   << m_ytiles;
            String assetName( stream.str() );
            
            AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( coordinates, assetName ) );
            assetLoader->open();
            Assets::ANSIFile ansiFile( *assetLoader );

            m_terminal->consumeNext( m_yborder + ( y * m_yheight ), m_xborder + ( x * m_xwidth ) );
            m_terminal->consumeAsset( ansiFile, 0, ansiFile.dataSize(), m_terminal->width(), m_xborder + ( x * m_xwidth ), Terminal::noMaxRow, Terminal::scrollLock, Terminal::transient );

            delete( assetLoader );

            drawGrid( x, y, false );
        }
    }
}

void MiniMap::drawGrid( int gridx, int gridy, bool selected )
{
    int attributes( 0 );
    if( selected )
    {
        attributes = attributesSelected;
    }
    else
    {
        attributes = attributesNotSelected;
    }

    m_terminal->consumeNext( m_yborder + ( gridy * m_yheight ), m_xborder + ( gridx * m_xwidth ), attributes );
    for( int x = 0; x < m_xwidth; ++x )
    {
        if( x == 0 )
        {
            m_terminal->consumeChar( '\xda', Terminal::scrollLock, Terminal::transient | Terminal::blit );
        }
        else if( x == ( m_xwidth - 1 ) )
        {
            m_terminal->consumeChar( '\xbf', Terminal::scrollLock, Terminal::transient | Terminal::blit );
        }
        else
        {
            m_terminal->consumeChar( '\xc4', Terminal::scrollLock, Terminal::transient | Terminal::blit );
        }
    }

    for( int y = 1; y < ( m_yheight - 1 ); ++y )
    {
        m_terminal->consumeNext( m_yborder + ( gridy * m_yheight ) + y, m_xborder + ( gridx * m_xwidth ), attributes );
        m_terminal->consumeChar( '\xb3', Terminal::scrollLock, Terminal::transient | Terminal::blit );
        m_terminal->consumeNext( m_yborder + ( gridy * m_yheight ) + y, m_xborder + ( gridx * m_xwidth ) + ( m_xwidth - 1 ), attributes );
        m_terminal->consumeChar( '\xb3', Terminal::scrollLock, Terminal::transient | Terminal::blit );
    }

    m_terminal->consumeNext( m_yborder + ( gridy * m_yheight ) + ( m_yheight - 1 ), m_xborder + ( gridx * m_xwidth ), attributes );
    for( int x = 0; x < m_xwidth; ++x )
    {
        if( x == 0 )
        {
            m_terminal->consumeChar( '\xc0', Terminal::scrollLock, Terminal::transient | Terminal::blit );
        }
        else if( x == ( m_xwidth - 1 ) )
        {
            m_terminal->consumeChar( '\xd9', Terminal::scrollLock, Terminal::transient | Terminal::blit );
        }
        else
        {
            m_terminal->consumeChar( '\xc4', Terminal::scrollLock, Terminal::transient | Terminal::blit );
        }
    }
}

World::Coordinates MiniMap::coordsAtOffset( int x, int y )
{
    int xhalf( ::floor( m_xtiles / 2 ) );
    int yhalf( ::floor( m_ytiles / 2 ) );
    return( World::Coordinates( m_centreCoordinates.m_worldID,
                                m_centreCoordinates.m_x - xhalf + x,
                                m_centreCoordinates.m_y + yhalf - y ) );
}

void MiniMap::drawHotkeys()
{
    m_hotkeys.clear();
    m_hotkeys.show( "\x18\x19\x1B\x1A", "Select" );
    m_hotkeys.show( "+", "Zoom in" );
    m_hotkeys.show( "-", "Zoom out" );
    m_hotkeys.show( "Ret", "Tport" );
    m_hotkeys.show( "Esc", "Close" );
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
