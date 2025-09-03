#include "String.h"
#include "TabBar.h"
#include "Terminal.h"
#include "WindowManager.h"

namespace Agape
{

namespace UI
{

TabBar::TabBar( WindowManager& windowManager,
                const String& windowName ) :
  m_terminal( nullptr )
{
    WindowManager::TerminalWindow terminalWindow;
    if( windowManager.getTerminalWindow( windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }
}

void TabBar::create( const String& name,
                     int reservedWidth,
                     enum Alignment alignment,
                     bool visible,
                     const String& text,
                     char attributes )
{
    struct Tab tab;
    tab.m_name = name;
    tab.m_reservedWidth = reservedWidth;
    tab.m_alignment = alignment;
    tab.m_visible = visible;
    tab.m_text = text;
    tab.m_attributes = attributes;
    m_tabs.push_back( tab );
    redrawAll();
}

void TabBar::remove( const String& name )
{
    Vector< struct Tab >::iterator it( m_tabs.begin() );
    for( ; it != m_tabs.end(); ++it )
    {
        struct Tab& tab( *it );
        if( tab.m_name == name )
        {
            m_tabs.erase( it );
            redrawAll();
            break;
        }
    }
}

bool TabBar::haveTab( const String& name )
{
    Vector< struct Tab >::iterator it( m_tabs.begin() );
    for( ; it != m_tabs.end(); ++it )
    {
        struct Tab& tab( *it );
        if( tab.m_name == name )
        {
            return true;
        }
    }

    return false;
}

void TabBar::update( const String& name,
                     const String& text,
                     char attributes )
{
    Vector< struct Tab >::iterator it( m_tabs.begin() );
    for( ; it != m_tabs.end(); ++it )
    {
        struct Tab& tab( *it );
        if( tab.m_name == name )
        {
            tab.m_text = text;
            if( attributes != 0 ) tab.m_attributes = attributes;
            redrawOne( name );
            break;
        }
    }
}

void TabBar::setVisible( const String& name,
                         bool visible )
{
    Vector< struct Tab >::iterator it( m_tabs.begin() );
    for( ; it != m_tabs.end(); ++it )
    {
        struct Tab& tab( *it );
        if( tab.m_name == name )
        {
            tab.m_visible = visible;
            redrawAll();
            break;
        }
    }
}

void TabBar::redrawAll()
{
    if( !m_terminal ) return;

    m_terminal->clearScreen();
    String name;
    redrawOne( name );
}

void TabBar::redrawOne( const String& name )
{
    if( !m_terminal ) return;

    int col( 0 );
    Vector< struct Tab >::const_iterator it( m_tabs.begin() );
    for( ; it != m_tabs.end(); ++it )
    {
        const struct Tab& tab( *it );
        if( ( tab.m_alignment == left ) &&
            tab.m_visible )
        {
            if( name.empty() || ( tab.m_name == name ) )
            {
                doRedraw( tab, col );
            }
            col += tab.m_reservedWidth + 3;
        }
    }

    col = m_terminal->width();
    it = m_tabs.begin();
    for( ; it != m_tabs.end(); ++it )
    {
        const struct Tab& tab( *it );
        if( ( tab.m_alignment == right ) &&
            tab.m_visible )
        {
            col -= tab.m_reservedWidth + 3;
            if( name.empty() || ( tab.m_name == name ) )
            {
                doRedraw( tab, col + 1 );
            }
        }
    }
}

void TabBar::doRedraw( const struct Tab& tab, int col )
{
    int padSize( tab.m_reservedWidth - tab.m_text.length() + 2 );
    String padLeft( ( padSize / 2 ) + ( padSize % 2 ), ' ' );
    String padRight( padSize / 2, ' ' );
    m_terminal->consumeNext( 0, col, tab.m_attributes );
    m_terminal->consumeString( padLeft + tab.m_text + padRight, Terminal::scrollLock );
}

} // namespace UI

} // namespace Agape
