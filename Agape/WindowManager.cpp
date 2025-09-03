#include "WindowManager.h"

#include "Collections.h"
#include "GraphicsDrivers/GraphicsDriver.h"
#include "Loggers/Logger.h"
#include "Utils/Cartesian.h"
#include "Utils/LiteStream.h"
#include "String.h"
#include "Terminal.h"

#include <math.h>

namespace Agape
{

WindowManager::TerminalWindow::TerminalWindow() :
  m_window( nullptr ),
  m_terminal( nullptr )
{
}

WindowManager::WindowManager( GraphicsDriver& graphicsDriver ) :
  m_graphicsDriver( graphicsDriver )
{
}

void WindowManager::createTerminalWindow( const TerminalWindow& terminalWindow )
{
    m_terminalWindows[terminalWindow.m_window->m_name] = terminalWindow;
}

bool WindowManager::getTerminalWindow( const String& name, TerminalWindow& terminalWindow )
{
    Map< String, TerminalWindow >::iterator it( m_terminalWindows.find( name ) );
    if( it != m_terminalWindows.end() )
    {
        terminalWindow = it->second;
        return true;
    }

    return false;
}

void WindowManager::setTerminalWindowVisible( const String& name, bool visible )
{
    Map< String, TerminalWindow >::iterator it( m_terminalWindows.find( name ) );
    if( it != m_terminalWindows.end() )
    {
        TerminalWindow& thisTerminalWindow( it->second );
        const GraphicsDriver::Window& thisWindow( *( thisTerminalWindow.m_window ) );

        if( thisWindow.m_visible != visible )
        {
            m_graphicsDriver.setWindowVisible( thisWindow.m_name, visible );

            if( visible )
            {
                thisTerminalWindow.m_terminal->repaint();
            }
            else
            {
                repaintTopmost( thisWindow.m_rect );
            }
        }
    }
}

void WindowManager::moveWindow( const String& name, const Point& origin )
{
    Map< String, TerminalWindow >::iterator it( m_terminalWindows.find( name ) );
    if( it != m_terminalWindows.end() )
    {
        TerminalWindow& thisTerminalWindow( it->second );
        const GraphicsDriver::Window& thisWindow( *( thisTerminalWindow.m_window ) );

        Rectangle startRect( thisWindow.m_rect );
        Rectangle endRect( origin,
                           thisWindow.m_rect.height(),
                           thisWindow.m_rect.width() );

#ifdef LOG_WINDOWS
        LOG_DEBUG( "Move start rect " + startRect.dump() );
        LOG_DEBUG( "Move end rect " + endRect.dump() );
#endif

        m_graphicsDriver.moveWindow( name, origin );

        Rectangle repaintRect( endRect.findUnionBoundingBox( startRect ) );
#ifdef LOG_WINDOWS
        LOG_DEBUG( "Window moved. Repainting area " + repaintRect.dump() );
#endif
        repaintTopmost( repaintRect );
    }
}

void WindowManager::repaintTopmost( const Rectangle& repaintRect )
{
    Vector< const GraphicsDriver::Window* > topMostWindows( m_graphicsDriver.findWindowsTopmostVisible( repaintRect ) );
    Vector< const GraphicsDriver::Window* >::const_iterator windowIt( topMostWindows.begin() );
    for( ; windowIt != topMostWindows.end(); ++windowIt )
    {
#ifdef LOG_WINDOWS
        LOG_DEBUG( "Repaint topmost window " + ( *windowIt )->m_name );
#endif
        Map< String, TerminalWindow >::iterator terminalWindowIt( m_terminalWindows.find( ( *windowIt )->m_name ) );
        if( terminalWindowIt != m_terminalWindows.end() )
        {
#ifdef LOG_WINDOWS
            LOG_DEBUG( "Found terminal window" );
#endif
            TerminalWindow& thisTerminalWindow( terminalWindowIt->second );
            const GraphicsDriver::Window& thisWindow( *( thisTerminalWindow.m_window ) );

            // Get redraw coords in window coordinate space
            int repaintX( repaintRect.originX() - thisWindow.m_rect.originX() );
            int repaintY( repaintRect.originY() - thisWindow.m_rect.originY() );

            int repaintWidth( repaintRect.width() );
            int repaintHeight( repaintRect.height() );

#ifdef LOG_WINDOWS
            {
            LiteStream stream;
            stream << "Redraw coords in window space: " << repaintX << "," << repaintY << " " << repaintHeight << "," << repaintWidth;
            LOG_DEBUG( stream.str() );
            }
#endif

            // Clamp to window size
            if( repaintX < 0 )
            {
                repaintWidth = repaintRect.width() + repaintX;
                repaintX = 0;
            }
            else if( ( repaintX + repaintRect.width() ) > thisWindow.m_rect.width() )
            {
                repaintWidth = thisWindow.m_rect.width() - repaintX;
            }

            if( repaintY < 0 )
            {
                repaintHeight = repaintRect.height() + repaintY;
                repaintY = 0;
            }
            else if( ( repaintY + repaintRect.height() ) > thisWindow.m_rect.height() )
            {
                repaintHeight = thisWindow.m_rect.height() - repaintY;
            }

#ifdef LOG_WINDOWS
            {
            LiteStream stream;
            stream << "Clamped redraw coords: " << repaintX << "," << repaintY << " " << repaintHeight << "," << repaintWidth;
            LOG_DEBUG( stream.str() );
            }
#endif

            // Determine redraw area in row, col coordinates
            // (may need to over-draw to ensure partially obscured
            // glyphs are redrawn).
            int startRow( ::floor( (double)repaintY / m_graphicsDriver.glyphHeight() ) );
            int startCol( ::floor( (double)repaintX / m_graphicsDriver.glyphWidth() ) );
            int endRow( ::ceil( (double)( repaintY + repaintHeight ) / m_graphicsDriver.glyphHeight() ) );
            int endCol( ::ceil( (double)( repaintX + repaintWidth ) / m_graphicsDriver.glyphWidth() ) );

            // Clamp again.
            if( startRow < 0 ) { startRow = 0; }
            if( startCol < 0 ) { startCol = 0; }
            if( endRow >= thisTerminalWindow.m_terminal->height() ) { endRow = thisTerminalWindow.m_terminal->height() - 1; }
            if( endCol >= thisTerminalWindow.m_terminal->width() ) { endCol = thisTerminalWindow.m_terminal->width() - 1; }

#ifdef LOG_WINDOWS
            {
            LiteStream stream;
            stream << "Redraw row/col coords: " << startRow << "," << startCol << " " << ( endRow - startRow ) + 1 << "," << ( endCol - startCol ) + 1;
            LOG_DEBUG( stream.str() );
            }
#endif

            thisTerminalWindow.m_terminal->repaint( startRow, startCol, ( endRow - startRow ) + 1, ( endCol - startCol ) + 1 );
        }
#ifdef LOG_WINDOWS
        else
        {
            LOG_DEBUG( "Terminal window not found!" );
        }
#endif
    }
}

} // namespace Agape