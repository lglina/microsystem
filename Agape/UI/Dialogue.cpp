#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "Dialogue.h"
#include "String.h"
#include "Terminal.h"
#include "WindowManager.h"

namespace
{
    const int firstCol( 2 );
    const int maxWidth( 36 );
    const int titleRow( 0 );
    const int contentFirstRow( 1 );
    const int contentMaxHeight( 7 );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

Dialogue* Dialogue::s_instance( nullptr );

Dialogue::Dialogue( WindowManager& windowManager,
                    const String& windowName,
                    const String& normalBackgroundAssetName,
                    const String& successBackgroundAssetName,
                    const String& errorBackgroundAssetName ) :
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_normalBackgroundAssetName( normalBackgroundAssetName ),
  m_successBackgroundAssetName( successBackgroundAssetName ),
  m_errorBackgroundAssetName( errorBackgroundAssetName ),
  m_terminal( nullptr )
{
    s_instance = this;

    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }
}

void Dialogue::show( enum Type type )
{
    if( s_instance ) s_instance->_show( type );
}

void Dialogue::hide()
{
    if( s_instance ) s_instance->_hide();
}

void Dialogue::drawTitle( const String& title )
{
    if( s_instance ) s_instance->_drawTitle( title );
}

void Dialogue::drawMessage( const String& message )
{
    if( s_instance ) s_instance->_drawMessage( message );
}

void Dialogue::_show( enum Type type )
{
    m_type = type;
    if( m_terminal )
    {
        m_windowManager.setTerminalWindowVisible( m_windowName, true );
        drawBackground();
    }
}

void Dialogue::_hide()
{
    if( m_terminal )
    {
        m_windowManager.setTerminalWindowVisible( m_windowName, false );
    }
}

void Dialogue::_drawTitle( const String& title )
{
    if( m_terminal )
    {
        char attributes( 0x07 );
        if( m_type == normal )
        {
            attributes = 0x9F;
        }
        else if( m_type == success )
        {
            attributes = 0x7F;
        }
        else if( m_type == error )
        {
            attributes = 0xCF;
        }

        m_terminal->printFormatted( title,
                                    titleRow,
                                    firstCol,
                                    1, // Lines max.
                                    maxWidth,
                                    Terminal::hCentre,
                                    Terminal::noVCentre,
                                    attributes );
    }
}

void Dialogue::_drawMessage( const String& message )
{
    if( m_terminal )
    {
        char attributes( 0x07 );
        if( m_type == normal )
        {
            attributes = 0x17;
        }
        else if( m_type == success )
        {
            attributes = 0x2F;
        }
        else if( m_type == error )
        {
            attributes = 0x47;
        }

        // Need preserve background so we don't get black background when using
        // [0m in a message, but we have no backing buffer for the small
        // dialogue box, so use blit to keep background colour instead.
        m_terminal->printFormatted( message,
                                    contentFirstRow,
                                    firstCol,
                                    contentMaxHeight,
                                    maxWidth,
                                    Terminal::hCentre,
                                    Terminal::vCentre,
                                    attributes,
                                    Terminal::blit );
        
        m_terminal->flush();
    }
}

void Dialogue::drawBackground()
{
    String backgroundAssetName;
    if( m_type == normal )
    {
        backgroundAssetName = m_normalBackgroundAssetName;
    }
    else if( m_type == success )
    {
        backgroundAssetName = m_successBackgroundAssetName;
    }
    else if( m_type == error )
    {
        backgroundAssetName = m_errorBackgroundAssetName;
    }

    if( !backgroundAssetName.empty() )
    {
        AssetLoaders::Baked assetLoader( World::Coordinates(), backgroundAssetName );
        if( assetLoader.open() )
        {
            Assets::ANSIFile ansiFile( assetLoader );
            m_terminal->consumeNext( 0, 0 );
            m_terminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
        }
    }
}

} // namespace UI

} // namespace Agape
