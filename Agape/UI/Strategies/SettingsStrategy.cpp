#include "InputDevices/InputDevice.h"
#include "UI/Forms/Form.h"
#include "UI/Strategies/EnterKeyStrategy.h"
#include "UI/Strategy.h"
#include "Collections.h"
#include "SettingsStrategy.h"
#include "StrategyHelper.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"
#include "WindowManager.h"

using Agape::String;

namespace
{
    const int contentFirstRow( 6 );
    const int contentCol( 8 );
    const int buttonSelected( 0x9F );
    const int buttonNotSelected( 0x97 );

    const String textAssetName( "settings-text" );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

Settings::Settings( InputDevice& inputDevice,
                    WindowManager& windowManager,
                    const String& windowName ) :
  m_inputDevice( inputDevice ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_completed( false ),
  m_calling( false ),
  m_currentForm( nullptr ),
  m_terminal( nullptr )
{
    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }
}

Settings::~Settings()
{
    closeForm();
}

void Settings::enter( const Value& parameters )
{
    m_completed = false;
    m_nextStrategy.clear();
    drawMenu();
}

void Settings::returnTo( const Value& parameters )
{
    m_calling = false;
    m_nextStrategy.clear();
    m_callingParameters = Value();
    drawMenu();
}

bool Settings::calling( String& strategyName, Value& parameters )
{
    if( m_calling )
    {
        strategyName = m_nextStrategy;
        parameters = m_callingParameters;
        return true;
    }

    return false;
}

bool Settings::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        closeForm();
        nextStrategy = m_nextStrategy;
        return true;
    }

    return false;
}

void Settings::run()
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
            if( m_currentForm != nullptr )
            {
                const Forms::Field& currentField( m_currentForm->currentField() );
                if( currentField.m_name == _WiFi_Networks )
                {
                    m_nextStrategy = "connectWiFi";
                    m_callingParameters = Value();
                    m_callingParameters[_title] = _settings;
                    m_calling = true;
                }
                else if( currentField.m_name == _Account_Key )
                {
                    m_nextStrategy = "enterKey";
                    m_callingParameters = Value();
                    m_callingParameters[_title] = _settings;
                    m_callingParameters[_keyType] = Strategies::EnterKey::kAccount;
                    m_calling = true;
                }
                else if( currentField.m_name == _Device_Key )
                {
                    m_nextStrategy = "enterKey";
                    m_callingParameters = Value();
                    m_callingParameters[_title] = _settings;
                    m_callingParameters[_keyType] = Strategies::EnterKey::kDevice;
                    m_calling = true;
                }
            }
        }
        else if( c == '\x1b' )
        {
            m_completed = true;
        }
        else if( m_currentForm != nullptr )
        {
            m_currentForm->consumeChar( c );
        }
    }
}

void Settings::drawBackground()
{
    if( m_terminal != nullptr )
    {
        Helper::drawMenuBackground( m_terminal, textAssetName );
    }
}

void Settings::drawMenu()
{
    drawBackground();

    Vector< Forms::Field > fields;
    fields.push_back( Forms::Field( _WiFi_Networks,
                                    contentFirstRow,
                                    contentCol,
                                    buttonNotSelected,
                                    buttonSelected ) );
    fields.push_back( Forms::Field( _Account_Key,
                                    contentFirstRow + 2,
                                    contentCol,
                                    buttonNotSelected,
                                    buttonSelected ) );
    fields.push_back( Forms::Field( _Device_Key,
                                    contentFirstRow + 4,
                                    contentCol,
                                    buttonNotSelected,
                                    buttonSelected ) );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                     m_windowName,
                                     String(),
                                     fields,
                                     Forms::Title() );

    m_currentForm->openUI();
    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->reset();
        m_currentForm->draw();
    }
}

void Settings::closeForm()
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
