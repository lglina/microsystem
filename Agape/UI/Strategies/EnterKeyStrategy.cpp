#include "InputDevices/InputDevice.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "UI/Forms/Utils/BIP39FormUtil.h"
#include "UI/Forms/Form.h"
#include "UI/Dialogue.h"
#include "ConfigurationStore.h"
#include "EnterKeyStrategy.h"
#include "KeyUtilities.h"
#include "StrategyHelper.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"

using Agape::String;
using namespace Agape::InputDevices;

namespace
{
    const int contentFirstRow( 6 );
    const int contentFirstCol( 8 );
    const int contentMaxWidth( 66 );
    const int contentAttributes( 0x07 );

    const int fieldAttributes( 0x07 );
    const int textWidth( 50 );
    const int textHeight( 1 );
    
    const int keyWordWidth( 8 );
    const int keyWordAttributes( 0x0F );

    const int keyFieldWidth( 8 );
    const int keyFieldHeight( 1 );

    const String welcomeTextAssetName( "welcome-text" );
    const String settingsTextAssetName( "settings-text" );

    const int successTime( 2000 ); // ms
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

const char* EnterKey::kAccount( "account" );
const char* EnterKey::kDevice( "device" );

EnterKey::EnterKey( WindowManager& windowManager,
            const String& windowName,
            InputDevice& inputDevice,
            KeyUtilities& keyUtilities,
            ConfigurationStore& configurationStore,
            Dialogue& dialogue,
            Timers::Factory& timerFactory ) :
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_inputDevice( inputDevice ),
  m_keyUtilities( keyUtilities ),
  m_configurationStore( configurationStore ),
  m_dialogue( dialogue ),
  m_timer( timerFactory.makeTimer() ),
  m_state( none ),
  m_completed( false ),
  m_currentForm( nullptr ),
  m_terminal( nullptr )
{
}

EnterKey::~EnterKey()
{
    closeForm();
    delete( m_timer );
}

void EnterKey::enter( const Value& parameters )
{
    m_completed = false;
    m_returnParameters = Value();
    m_parameters = parameters;

    if( m_parameters.hasValue( _keyType ) &&
        ( ( m_parameters[_keyType] == String( kAccount ) ) ||
          ( m_parameters[_keyType] == String( kDevice ) ) ) )
    {
        WindowManager::TerminalWindow terminalWindow;
        if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
        {
            m_terminal = terminalWindow.m_terminal;
            m_state = enterKey;
            drawEnterKeyForm();
        }
        else
        {
            m_returnParameters[_success] = 0;
            m_completed = true; // Uh oh!
        }
    }
    else
    {
        m_returnParameters[_success] = 0;
        m_completed = true;
    }
}

void EnterKey::returnTo( const Value& parameters )
{
}

bool EnterKey::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool EnterKey::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        closeForm();
        parameters = m_returnParameters;
        return true;
    }

    return false;
}

void EnterKey::run()
{
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }

        switch( m_state )
        {
        case enterKey:
            if( c == '\n' )
            {
                if( m_currentForm != nullptr )
                {
                    char key[16];
                    if( getKeyFromForm( key ) )
                    {
                        m_state = success;
                        setKey( key );
                        drawSuccess();
                        m_timer->reset();
                    }
                    else
                    {
                        m_state = error;
                        drawError();
                    }
                }
            }
            else if( c == '\x1b' )
            {
                m_returnParameters[_success] = 0;
                m_completed = true;
            }
            else if( m_currentForm != nullptr )
            {
                m_currentForm->consumeChar( c );
                if( c != Key::tab && c != Key::shiftTab && c != Key::backspace )
                {
                    autocompleteKeyword();
                }
            }
            break;
        case error:
            if( c == '\n' )
            {
                m_state = enterKey;
                hideDialogue();
            }
            break;
        default:
            break;
        }
    }

    if( ( m_state == success ) && ( m_timer->ms() >= successTime ) )
    {
        hideDialogue();
        m_returnParameters[_success] = 1;
        m_completed = true;
    }
}

void EnterKey::drawBackground()
{
    if( m_terminal != nullptr )
    {
        String textAssetName;
        if( m_parameters[_title] == String( _welcome ) )
        {
            textAssetName = welcomeTextAssetName;
        }
        else if( m_parameters[_title] == String( _settings ) )
        {
            textAssetName = settingsTextAssetName;
        }

        Helper::drawMenuBackground( m_terminal, textAssetName );
    }
}

void EnterKey::drawEnterKeyForm()
{
    drawBackground();

    const char* message1( "Enter your twelve word " );
    const char* message2( " key below. You only need to type the first few letters of each word\
                          and we'll guess the rest. When you're done, hit \x1b[97mRet\x1b[0m. Hit \x1b[97mTab\x1b[0m or\
                          \x1b[97mSh-Tab\x1b[0m to move between words if needed. If you make a mistake, hit \x1b[97mBs\x1b[0m.\
                          To cancel, hit \x1b[97mEsc\x1b[0m." );
    String message( message1 );
    if( m_parameters[_keyType] == String( kAccount ) )
    {
        message += "\x1b[97maccount\x1b[0m";
    }
    else
    {
        message += "\x1b[97mdevice\x1b[0m";
    }
    message += message2;
    
    m_terminal->printFormatted( message,
                                contentFirstRow,
                                contentFirstCol,
                                4, // Lines max.
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );

    Vector< Forms::Field > fields;

    // N.B. Can't display existing account key as we don't store it
    // (only the derived keys).
    bool haveKey( false );
    if( m_parameters[_keyType] == String( kDevice ) )
    {
        haveKey = m_configurationStore.hasKey( _deviceAuthKey );
    }

    if( haveKey )
    {
        String key = m_configurationStore.get( _deviceAuthKey );
        Forms::Utils::BIP39::createKeyForm( fields,
                                            contentFirstRow + 5,
                                            contentFirstCol,
                                            fieldAttributes,
                                            &key[0] );
    }
    else
    {
        Forms::Utils::BIP39::createKeyForm( fields,
                                            contentFirstRow + 5,
                                            contentFirstCol,
                                            fieldAttributes );
    }

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager, m_windowName, String(), fields, Forms::Title() );
    m_currentForm->openUI();

    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->draw();
    }
}

void EnterKey::drawSuccess()
{
    const char* message( "Key accepted" );
    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( message );
}

void EnterKey::drawError()
{
    const char* message( "The key you have entered is invalid. Please check that\
                          you have entered the correct words.\
                          Hit \x1b[97mRet\x1b[0m to go back." );
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void EnterKey::hideDialogue()
{
    m_dialogue.hide();
}

void EnterKey::closeForm()
{
    if( m_currentForm != nullptr )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

bool EnterKey::getKeyFromForm( char* key )
{
    if( m_currentForm != nullptr )
    {
        return Forms::Utils::BIP39::getKey( m_currentForm, key );
    }

    return false;
}

void EnterKey::setKey( char* key )
{
    if( m_parameters[_keyType] == String( kAccount ) )
    {
        String accountSubKey( m_keyUtilities.accountSubKeySize(), '\0' );
        String accountAuthKey( m_keyUtilities.accountSubKeySize(), '\0' );
        m_keyUtilities.getAccountSubKey( key, &accountSubKey[0] );
        m_keyUtilities.getAccountAuthKey( key, &accountAuthKey[0] );
        m_configurationStore.get( _accountSubKey ) = accountSubKey;
        m_configurationStore.get( _accountAuthKey ) = accountAuthKey;
    }
    else if( m_parameters[_keyType] == String( kDevice ) )
    {
        m_configurationStore.get( _deviceAuthKey ) = String( key, 16 );
    }
    m_configurationStore.save();
}

void EnterKey::autocompleteKeyword()
{
    if( m_currentForm != nullptr )
    {
        Forms::Utils::BIP39::tryAutoComplete( m_currentForm );
    }
}

} // namespace UI

} // namespace Strategies

} // namespace Agape
