#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "InputDevices/InputDevice.h"
#include "Lines/Line.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "UI/Forms/Form.h"
#include "UI/Dialogue.h"
#include "Utils/StrToHex.h"
#include "Collections.h"
#include "ConnectWiFi.h"
#include "StrategyHelper.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"

using Agape::String;

namespace
{
    const int contentFirstRow( 6 );
    const int contentCol( 8 );
    const int contentMaxWidth( 66 );
    const int contentAttributes( 0x07 );

    const int labelAttributes( 0x07 );

    const int selectWidth( 50 );
    const int selectHeight( 8 );
    const int fieldAttributes( 0x07 );
    const int selectionAttributes( 0x4F );
    const int textWidth( 50 );
    const int textHeight( 1 );

    const int guideRow( 20 );

    const char* welcomeTextAssetName( "welcome-text" );
    const char* settingsTextAssetName( "settings-text" );

    const int connectTimeout( 10000 ); // ms.
    const int scanWaitTime( 2000 ); // ms.
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

ConnectWiFi::ConnectWiFi( InputDevice& inputDevice,
                          WindowManager& windowManager,
                          const String& windowName,
                          Line& line,
                          Timers::Factory& timerFactory,
                          Dialogue& dialogue ) :
  m_inputDevice( inputDevice ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_line( line ),
  m_dialogue( dialogue ),
  m_timer( timerFactory.makeTimer() ),
  m_state( none ),
  m_completed( false ),
  m_terminal( nullptr ),
  m_currentForm( nullptr ),
  m_addOnly( false )
{
}

ConnectWiFi::~ConnectWiFi()
{
    closeForm();
    delete( m_timer );
}

void ConnectWiFi::enter( const Value& parameters )
{
    m_completed = false;
    m_parameters = parameters;

    m_line.open();

    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;

        if( parameters[_mode] == String( _add ) )
        {
            m_addOnly = true;

            m_state = scanning;
            drawScanning();

            m_timer->reset();
        }
        else
        {
            m_state = list;
            drawList();
        }
    }
    else
    {
        m_returnParameters[_success] = 0;
        m_completed = true; // Uh oh!
    }
}

void ConnectWiFi::returnTo( const Value& parameters )
{
}

bool ConnectWiFi::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool ConnectWiFi::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        closeForm();
        parameters = m_returnParameters;
        return true;
    }

    return false;
}

void ConnectWiFi::run()
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
        case list:
            if( c == 'a' )
            {
                m_state = scanning;
                drawScanning();

                m_timer->reset();
            }
            else if( c == 'x' )
            {
                deleteNetwork();
                drawList();
            }
            else if( c == '\x1b' )
            {
                m_completed = true;
            }
            else if( m_currentForm != nullptr )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        case network:
            if( c == '\n' )
            {
                setNetwork();

                m_state = password;
                drawPassword();
            }
            else if( c == 'r' )
            {
                m_state = scanning;
                drawScanning();

                m_timer->reset();
            }
            else if( c == '\x1b' )
            {
                if( !m_addOnly )
                {
                    m_state = list;
                    drawList();
                }
                else
                {
                    m_returnParameters[_success] = 0;
                    m_completed = true;
                }
            }
            else if( m_currentForm != nullptr )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        case password:
            if( c == '\n' )
            {
                setPassword();

                m_state = pending;
                drawPendingConnect();

                connect();

                m_timer->reset();
            }
            else if( c == '\x1b' )
            {
                m_state = network;
                drawNetwork();
            }
            else if( m_currentForm != nullptr )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        case error:
            if( c == '\n' )
            {
                m_state = password;
                hideDialogue();
            }
            break;
        default:
            break;
        }
    }

    if( m_state == scanning )
    {
        if( m_timer->ms() >= scanWaitTime )
        {
            m_state = network;
            drawNetwork();
        }
    }
    else if( m_state == pending )
    {
        Line::LineStatus lineStatus( m_line.getLineStatus() );

        if( lineStatus.m_ready )
        {
            m_state = success;
            drawSuccess();
            m_timer->reset();
        }
        else if( m_timer->ms() >= connectTimeout )
        {
            m_state = error;
            drawError();
        }
    }
    else if( m_state == success )
    {
        if( m_timer->ms() >= 2000 )
        {
            hideDialogue();

            if( m_addOnly )
            {
                m_returnParameters[_success] = 1;
                m_completed = true;
            }
            else
            {
                m_state = list;
                drawList();
            }
        }
    }
}

void ConnectWiFi::drawBackground()
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

void ConnectWiFi::drawList()
{
    Vector< Line::ConfigOption > configOptions( m_line.getConfigOptions() );

    // FIXME: Use \x1f delim instead of semicolon? Would need to
    // modify line classes and modem firmware?
    String networks;
    Vector< Line::ConfigOption >::const_iterator it( configOptions.begin() );
    for( ; it != configOptions.end(); ++it )
    {
        if( it->m_name == _Access_Point_Names )
        {
            networks = it->m_alternatives;
            break;
        }
    }

    closeForm();
    drawBackground();

    m_terminal->consumeNext( guideRow, contentCol );
    m_terminal->consumeString( "\x1b[0;97;100mA\x1b[37m Add network\x1b[0m  \x1b[0;97;100mX\x1b[37m Delete network\x1b[0m  \x1b[97;100mEsc\x1b[37m Exit\x1b[0m\r\n", Terminal::scrollLock, Terminal::preserveBackground );

    Vector< Forms::Field > fields;
    Forms::Field field( _My_WiFi_Networks,
                        selectWidth,
                        selectHeight,
                        contentFirstRow,
                        contentCol,
                        fieldAttributes,
                        networks,
                        selectionAttributes,
                        labelAttributes );
    field.m_labelRow = contentFirstRow;
    field.m_labelCol = contentCol;
    fields.push_back( field );

    m_currentForm = new Forms::Form( m_windowManager, m_windowName, String(), fields, Forms::Title() );
    m_currentForm->openUI();

    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->draw();
    }
}

void ConnectWiFi::drawScanning()
{
    m_line.setConfigOption( _Access_Point_Scan, "scan" ); // Initiate scan.
    drawPendingScan();
}

void ConnectWiFi::drawNetwork()
{
    hideDialogue();

    closeForm();
    drawBackground();

    m_terminal->consumeNext( guideRow, contentCol );
    m_terminal->consumeString( "\x1b[0;97;100m\x18/\x19\x1b[37m Select\x1b[0m  \x1b[97;100mRet\x1b[37m Connect\x1b[0m  \x1b[97;100mR\x1b[37m Refresh\x1b[0m  \x1b[97;100mEsc\x1b[37m Back\x1b[0m", Terminal::scrollLock, Terminal::preserveBackground );

    Vector< Line::ConfigOption > options( m_line.getConfigOptions() );

    const char* message( "Use the arrow keys to select your WiFi network, then hit \x1b[97mRet\x1b[0m.\
                          If you don't see your network listed, hit \x1b[97mR\x1b[0m to refresh the list.\
                          Note: This computer doesn't work with networks that require you to use a web browser\
                          to log in, such as public WiFi networks." );
    m_terminal->printFormatted( message,
                                contentFirstRow,
                                contentCol,
                                Terminal::noMaxHeight,
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );

    Vector< Line::ConfigOption >::const_iterator it( options.begin() );
    for( ; it != options.end(); ++it )
    {
        if( it->m_name == _Access_Point_Scan )
        {
            break;
        }
    }

    if( it != options.end() )
    {
        Vector< Forms::Field > fields;
        fields.push_back( Forms::Field( _Network,
                                        selectWidth,
                                        selectHeight,
                                        contentFirstRow + 5,
                                        contentCol,
                                        fieldAttributes,
                                        it->m_alternatives,
                                        selectionAttributes ) );

        m_currentForm = new Forms::Form( m_windowManager, m_windowName, String(), fields, Forms::Title() );
        m_currentForm->openUI();

        if( m_currentForm->uiIsOpen() )
        {
            m_currentForm->draw();
        }
    }
}

void ConnectWiFi::drawPassword()
{
    closeForm();
    drawBackground();

    const char* message( "Now enter your WiFi password, then hit \x1b[97mRet\x1b[0m and we'll get you\
                          connected! If you make a mistake, hit \x1b[97mBs\x1b[0m. If you want to choose\
                          a different network, hit \x1b[97mEsc\x1b[0m." );

    m_terminal->printFormatted( message,
                                contentFirstRow,
                                contentCol,
                                Terminal::noMaxHeight,
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );

    Vector< Forms::Field > fields;
    fields.push_back( Forms::Field( _Password,
                                    textWidth,
                                    textHeight,
                                    contentFirstRow + 4,
                                    contentCol,
                                    fieldAttributes ) );

    m_currentForm = new Forms::Form( m_windowManager, m_windowName, String(), fields, Forms::Title() );
    m_currentForm->openUI();

    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->draw();
    }
}

void ConnectWiFi::drawPendingScan()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Please Wait" );
    m_dialogue.drawMessage( "Scanning for WiFi networks..." );
}

void ConnectWiFi::drawPendingConnect()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Please Wait" );
    m_dialogue.drawMessage( "Connecting to WiFi..." );
}

void ConnectWiFi::drawSuccess()
{
    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( "Connected successfully" );
}

void ConnectWiFi::drawError()
{
    const char* message( "We were unable to connect to your WiFi network.\
                          Please ensure the password is correct, and that\
                          you have selected the right network name.\
                          Hit \x1b[97mRet\x1b[0m to go back." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void ConnectWiFi::hideDialogue()
{
    m_dialogue.hide();
}

void ConnectWiFi::closeForm()
{
    if( m_currentForm != nullptr )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

void ConnectWiFi::deleteNetwork()
{
    if( m_currentForm != nullptr )
    {
        m_line.setConfigOption( _Delete_Access_Point_Name, m_currentForm->getFieldContents( _My_WiFi_Networks ) );
    }
}

void ConnectWiFi::setNetwork()
{
    if( m_currentForm != nullptr )
    {
        m_line.setConfigOption( _Add_Access_Point_Name, m_currentForm->getFieldContents( _Network ) );
    }
}

void ConnectWiFi::setPassword()
{
    if( m_currentForm != nullptr )
    {
        m_line.setConfigOption( _Add_Access_Point_Password, strToHex( m_currentForm->getFieldContents( _Password ) ) );
    }
}

void ConnectWiFi::connect()
{
    m_line.connect();
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
