#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "InputDevices/InputDevice.h"
#include "Lines/Line.h"
#include "UI/Forms/Form.h"
#include "Utils/StrToHex.h"
#include "Chooser.h"
#include "Collections.h"
#include "Menu.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"

#include "Warp.h"

using Agape::String;
using namespace Agape::InputDevices;

namespace
{
    const int contentFirstRow( 6 );
    const int contentCol( 8 );
    const int buttonSelected( 0x9F );
    const int buttonNotSelected( 0x97 );

    const String backgroundAssetName( "menu" );
    const String textAssetName( "menu-text" );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

Menu::Menu( InputDevice& inputDevice,
            WindowManager& windowManager,
            const String& windowName,
            Chooser& chooser,
            const String& statusWindowName ) :
  m_inputDevice( inputDevice ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_chooser( chooser ),
  m_calling( false ),
  m_completed( false ),
  m_autoEnter( true ),
  m_currentForm( nullptr ),
  m_terminal( nullptr ),
  m_statusTerminal( nullptr )
{
    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }

    if( windowManager.getTerminalWindow( statusWindowName, terminalWindow ) )
    {
        m_statusTerminal = terminalWindow.m_terminal;
    }
}

Menu::~Menu()
{
    closeForm();
}

void Menu::enter( const Value& parameters )
{
    m_nextStrategy.clear();
    m_completed = false;
    m_returnParameters = Value();

#ifdef __EMSCRIPTEN__
    m_nextStrategy = _tela;
#else
    m_nextStrategy = _onboarding;
#endif
    m_calling = true;
}

void Menu::returnTo( const Value& parameters )
{
    m_calling = false;

    if( m_nextStrategy == _tela )
    {
        m_nextStrategy = _onboarding;
        m_calling = true;
    }
    else if( ( m_nextStrategy == _onboarding ) && m_autoEnter )
    {
        m_nextStrategy = _enterWorld;
        m_calling = true;
    }
    else if( m_nextStrategy == _enterWorld )
    {
        m_autoEnter = false;
    }

    if( !m_calling )
    {
        m_nextStrategy.clear();
        m_callingParameters = Value();
        
        drawMenu();
        //drawChooserState(); // FIXME: Needs to be converted to tab bar entry?
    }
}

bool Menu::calling( String& strategyName, Value& parameters )
{
    if( m_calling )
    {
        strategyName = m_nextStrategy;
        parameters = m_callingParameters;
        return true;
    }

    return false;
}

bool Menu::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        nextStrategy = m_nextStrategy;
        parameters = m_returnParameters;
        return true;
    }

    return false;
}

void Menu::run()
{
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }

        if( c == '\n' )
        {
            if( m_currentForm )
            {
                const Forms::Field& currentField( m_currentForm->currentField() );
                if( currentField.m_name == _Enter_World )
                {
                    m_nextStrategy = "enterWorld";
                    m_callingParameters[_worldID] = _default;
                    m_calling = true;
                }
                else if( currentField.m_name == _Select_World )
                {
                    m_nextStrategy = "worldbook";
                    m_calling = true;
                }
                else if( currentField.m_name == _Select_Server )
                {
                    m_nextStrategy = "phonebook";
                    m_calling = true;
                }
                else if( currentField.m_name == _Settings )
                {
                    m_nextStrategy = "settings";
                    m_calling = true;
                }
                else if( currentField.m_name == _Credits )
                {
                    m_nextStrategy = "credits";
                    m_calling = true;
                }
            }
        }
        else if( c == control( 'c' ) )
        {
            m_chooser.nextChoice();
        }
        else if( c == control( 'z' ) )
        {
            m_nextStrategy = "splash";
            m_returnParameters[_next] = 1;
            m_completed = true;
        }
        else
        {
            if( m_currentForm )
            {
                m_currentForm->consumeChar( c );
            }
        }
    }
}

void Menu::drawBackground()
{
    if( m_terminal != nullptr )
    {
        Warp w1( "Clear screen" );
        m_terminal->clearScreen();
        w1.report();

        AssetLoaders::Baked backgroundAssetLoader( World::Coordinates(), backgroundAssetName );
        if( backgroundAssetLoader.open() )
        {
            Assets::ANSIFile ansiFile( backgroundAssetLoader );
            Warp w2( "Draw menu" );
            m_terminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
            w2.report();
        }

        AssetLoaders::Baked textAssetLoader( World::Coordinates(), textAssetName );
        if( textAssetLoader.open() )
        {
            Assets::ANSIFile ansiFile( textAssetLoader );
            m_terminal->consumeNext( 1, 5 );
            m_terminal->consumeAsset( ansiFile, 0, ansiFile.dataSize(), ansiFile.width(), 5, Terminal::noMaxRow, Terminal::scrollLock );
        }
    }
}

void Menu::drawMenu()
{
    drawBackground();

    if( m_currentForm == nullptr )
    {
        Vector< Forms::Field > fields;
        fields.push_back( Forms::Field( _Enter_World,
                                        contentFirstRow,
                                        contentCol,
                                        buttonNotSelected,
                                        buttonSelected ) );
        fields.push_back( Forms::Field( _Select_World,
                                        contentFirstRow + 2,
                                        contentCol,
                                        buttonNotSelected,
                                        buttonSelected ) );
        fields.push_back( Forms::Field( _Select_Server,
                                        contentFirstRow + 4,
                                        contentCol,
                                        buttonNotSelected,
                                        buttonSelected ) );
        fields.push_back( Forms::Field( _Settings,
                                        contentFirstRow + 6,
                                        contentCol,
                                        buttonNotSelected,
                                        buttonSelected ) );
        fields.push_back( Forms::Field( _Credits,
                                        contentFirstRow + 8,
                                        contentCol,
                                        buttonNotSelected,
                                        buttonSelected ) );

        closeForm();

        m_currentForm = new Forms::Form( m_windowManager, m_windowName, String(), fields, Forms::Title() );

        m_currentForm->openUI();

        if( m_currentForm->uiIsOpen() )
        {
            m_currentForm->draw();
        }
    }
    else
    {
        m_currentForm->draw();
    }
}

void Menu::drawChooserState()
{
    if( m_statusTerminal )
    {
        String currentChoice( m_chooser.currentChoiceName() );
        m_statusTerminal->consumeNext( 0,
                                       m_statusTerminal->width() - currentChoice.length(),
                                       Terminal::attributes( Terminal::colMagenta, Terminal::colWhite ) );
        m_statusTerminal->consumeString( currentChoice, Terminal::scrollLock, Terminal::preserveBackground );
    }
}

void Menu::closeForm()
{
    if( m_currentForm != nullptr )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
