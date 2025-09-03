#include "Hotkeys.h"
#include "String.h"
#include "Terminal.h"
#include "WindowManager.h"

namespace
{
    const int keyBGColour( Agape::Terminal::colDarkGrey );
    const int keyFGColour( Agape::Terminal::colBlack );
    const int nameBGColour( Agape::Terminal::colBlack );
    const int nameFGColour( Agape::Terminal::colDarkGrey );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

Hotkeys::Hotkeys( WindowManager& windowManager,
                  const String& windowName ) :
  m_terminal( nullptr )
{
    WindowManager::TerminalWindow terminalWindow;
    if( windowManager.getTerminalWindow( windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }
}

void Hotkeys::clear()
{
    if( m_terminal )
    {
        m_terminal->clearScreen();
    }
}

void Hotkeys::show( const String& keys, const String& name )
{
    if( m_terminal )
    {
        m_terminal->setAttributes( Terminal::attributes( keyBGColour, keyFGColour ) );
        m_terminal->consumeString( keys, Terminal::scrollLock, Terminal::literal );
        m_terminal->setAttributes( Terminal::attributes( nameBGColour, nameFGColour ) );
        m_terminal->consumeString( name, Terminal::scrollLock, Terminal::literal );
        if( ( keys.length() + name.length() ) < m_terminal->width() )
        {
            m_terminal->consumeString( "\r\n" );
        }
    }
}

void Hotkeys::spacer()
{
    if( m_terminal )
    {
        m_terminal->consumeString( "\r\n" );
    }
}

} // namespace UI

} // namespace Agape
