#include "Loggers/Logger.h"
#include "Utils/Cartesian.h"
#include "Collections.h"
#include "GraphicsDriver.h"
#include "String.h"

namespace Agape
{

const bool GraphicsDriver::transparent( true );

GraphicsDriver::~GraphicsDriver()
{
    Vector< Window* >::iterator it;
    for( it = m_windows.begin(); it != m_windows.end(); ++it )
    {
        delete( *it );
    }
}

const GraphicsDriver::Window* GraphicsDriver::createWindow( const Window& window )
{
    Window* newWindow( new Window( window ) );
    m_windows.push_back( newWindow );
    return newWindow;
}

void GraphicsDriver::setWindowVisible( const String& windowName, bool visible )
{
    Vector< Window* >::iterator it;
    for( it = m_windows.begin(); it != m_windows.end(); ++it )
    {
        Window* currentWindow( *it );
        if( currentWindow->m_name == windowName )
        {
            currentWindow->m_visible = visible;
        }
    }
}

bool GraphicsDriver::findWindow( const String& windowName, const Window*& window ) const
{
    Vector< Window* >::const_iterator it;
    for( it = m_windows.begin(); it != m_windows.end(); ++it )
    {
        const Window* currentWindow( *it );
        if( currentWindow->m_name == windowName )
        {
            window = currentWindow;
            return true;
        }
    }

    return false;
}

void GraphicsDriver::moveWindow( const String& windowName, const Point& origin )
{
    Vector< Window* >::iterator it;
    for( it = m_windows.begin(); it != m_windows.end(); ++it )
    {
        Window* currentWindow( *it );
        if( currentWindow->m_name == windowName )
        {
            currentWindow->m_rect.translate( origin );
            break;
        }
    }
}

Vector< const GraphicsDriver::Window* > GraphicsDriver::findWindowsTopmostVisible( const Rectangle& drawRect ) const
{
    Vector< const Window* > topmost;
    Vector< Window* >::const_iterator it;
    for( it = m_windows.begin(); it != m_windows.end(); ++it )
    {
        const Window* currentWindow( *it );
        if( currentWindow->m_visible &&
            drawRect.intersects( currentWindow->m_rect ) )
        {
            topmost.push_back( currentWindow );
        }
    }

    return topmost;
}

bool GraphicsDriver::isWindowTopmostVisible( const Rectangle& drawRect, const String& windowName ) const
{
    bool isTopmost( false );

    Vector< Window* >::const_iterator it;
    for( it = m_windows.begin(); it != m_windows.end(); ++it )
    {
        const Window* currentWindow( *it );
#ifdef LOG_WINDOWS
        //LOG_DEBUG( "Draw at " + drawRect.dump() + " from " + windowName + " intersects " + currentWindow->m_name + " with rect " + currentWindow->m_rect.dump() + "?" );
#endif
        if( currentWindow->m_visible &&
            drawRect.intersects( currentWindow->m_rect ) )
        {
            // Graphics driver will reject the draw if ANY of the draw
            // area intersects an upper window (front-most in the deque)
            // that isn't the draw window.
            if( currentWindow->m_name == windowName )
            {
                isTopmost = true;
            }
            else
            {
                isTopmost = false;
            }
        }
    }

#ifdef LOG_WINDOWS
    if( isTopmost )
    {
        //LOG_DEBUG( windowName + " IS topmost" );
    }
    else
    {
        //LOG_DEBUG( windowName + " IS NOT topmost" );
    }
#endif

    return isTopmost;
}

} // namespace Agape
