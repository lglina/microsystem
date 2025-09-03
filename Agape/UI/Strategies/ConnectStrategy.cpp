#include "InputDevices/InputDevice.h"
#include "Lines/Line.h"
#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "UI/Dialogue.h"
#include "ConfigurationStore.h"
#include "ConnectStrategy.h"
#include "Phonebook.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Value.h"

namespace
{
    const int readyTimeout( 10000 ); // ms.
    const int carrierTimeout( 10000 ); // ms.
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

Connect::Connect( InputDevice& inputDevice,
                  WindowManager& windowManager,
                  const String& windowName,
                  Phonebook& phonebook,
                  ConfigurationStore& configurationStore,
                  Line& line,
                  Timers::Factory& timerFactory,
                  Dialogue& dialogue ) :
  m_inputDevice( inputDevice ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_phonebook( phonebook ),
  m_configurationStore( configurationStore ),
  m_line( line ),
  m_timer( timerFactory.makeTimer() ),
  m_dialogue( dialogue ),
  m_state( none ),
  m_calling( false ),
  m_completed( false ),
  m_terminal( nullptr ),
  m_nonInteractive( false )
{
}

Connect::~Connect()
{
    delete( m_timer );
}

void Connect::enter( const Value& parameters )
{
    m_calling = false;
    m_callingParameters = Value();
    m_nextStrategy.clear();

    m_nonInteractive = ( (int)parameters[_nonInteractive] == 1 );

    LOG_DEBUG( "ConnectStrategy: Entering" );
    Line::LineStatus lineStatus( m_line.getLineStatus() );
    if( lineStatus.m_carrier )
    {
        // Don't re-connect!
        LOG_DEBUG( "ConnectStrategy: Line already has carrier." );
        m_completed = true;
    }
    else
    {
        LOG_DEBUG( "ConnectStrategy: No carrier. Checking for default phonebook entry." );
        m_completed = false;
        
        // FIXME: Could allow PhonebookStrategy to pass an entry name as a
        // parameter here, to allow connecting to something other than the
        // current default entry.
        if( m_phonebook.hasDefaultEntry() )
        {
            String defaultName;
            String defaultNumber;
            m_phonebook.getDefaultEntry( defaultName, defaultNumber );
            bool requiresAuthentication( m_phonebook.requiresAuthentication( defaultName ) );

            LOG_DEBUG( "ConnectStrategy: Checking if auth required." );
            if( ( m_configurationStore.hasKey( _accountAuthKey ) &&
                  m_configurationStore.hasKey( _deviceAuthKey ) ) ||
                !requiresAuthentication )
            {
                m_line.setRequiresAuthentication( requiresAuthentication );

                m_state = readyPending;
                drawReadyPending();

                LOG_DEBUG( "ConnectStrategy: Checking if line ready" );
                Line::LineStatus lineStatus( m_line.getLineStatus() );
                if( !lineStatus.m_ready )
                {
                    LOG_DEBUG( "ConnectStrategy: Line not ready. Opening." );
                    m_line.open();
                    LOG_DEBUG( "ConnectStrategy: Line not ready. Connecting." );
                    m_line.connect();
                    m_timer->reset();
                    LOG_DEBUG( "ConnectStrategy: Opened and connected." );
                }
            }
            else
            {
                m_state = error;
                drawKeysError();
            }
        }
        else
        {
            m_state = error;
            drawPhonebookError();
        }
    }
}

void Connect::returnTo( const Value& parameters )
{
    m_calling = false;
    m_callingParameters = Value();
    m_nextStrategy.clear();

    if( m_state == message )
    {
        m_returnParameters[_success] = 1;
        m_completed = true;
    }
}

bool Connect::calling( String& strategyName, Value& parameters )
{
    if( m_calling )
    {
        strategyName = m_nextStrategy;
        parameters = m_callingParameters;
        return true;
    }

    return false;
}

bool Connect::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        parameters = m_returnParameters;
        return true;
    }

    return false;
}

void Connect::run()
{
    //LOG_DEBUG( "ConnectStrategy: Run." );
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }
    
        switch( m_state )
        {
        case error:
            if( c == '\n' )
            {
                m_completed = true;
                m_returnParameters[_success] = 0;
                hideDialogue();
            }
            break;
        default:
            break;
        }
    }

    if( m_state == readyPending )
    {
        //LOG_DEBUG( "ConnectStrategy: ReadyPending." );
        Line::LineStatus lineStatus( m_line.getLineStatus() );

        if( lineStatus.m_ready )
        {
            LOG_DEBUG( "ConnectStrategy: Line ready. Waiting for carrier." );
            m_state = carrierPending;
            drawCarrierPending();

            dial();
            
            m_timer->reset();
        }
        else if( m_timer->ms() >= readyTimeout )
        {
            m_state = error;
            drawReadyError();
        }
    }
    else if( m_state == carrierPending )
    {
        Line::LineStatus lineStatus( m_line.getLineStatus() );

        if( lineStatus.m_carrier )
        {
            m_state = success;
            drawSuccess();
            m_timer->reset();
        }
        else if( m_timer->ms() >= carrierTimeout )
        {
            if( !m_nonInteractive )
            {
                m_state = error;
                drawCarrierError();
            }
            else
            {
                // Don't hold user here - let caller handle the failure.
                m_completed = true;
                m_returnParameters[_success] = 0;
                hideDialogue();
            }
        }
    }
    else if( m_state == success )
    {
        if( m_timer->ms() >= 2000 )
        {
            hideDialogue();

            if( !m_nonInteractive )
            {
                m_state = message;
                m_nextStrategy = _message;
                m_callingParameters[_assetName] = _servermessage;
                m_calling = true;
            }
            else
            {
                // Skip server MOTD
                m_returnParameters[_success] = 1;
                m_completed = true;
            }
        }
    }
}

void Connect::drawReadyPending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Please Wait" );
    m_dialogue.drawMessage( "Opening line..." );
}

void Connect::drawCarrierPending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Please Wait" );
    m_dialogue.drawMessage( "Dialling..." );
}

void Connect::drawSuccess()
{
    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( "Connected successfully" );
}

void Connect::drawPhonebookError()
{
    const char* message( "No server is configured. Please configure a server,\
                          or you will be unable to create, join or connect to any world.\
                          \x1b[97mRet\x1b[0m to go back." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void Connect::drawReadyError()
{
    const char* message( "Unable to connect to a WiFi network. Please ensure\
                          you are within range of one of your configured networks. Hit\
                          \x1b[97mRet\x1b[0m to go back." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void Connect::drawCarrierError()
{
    const char* message( "Unable to connect to the server. Please check your\
                          network connection, or try again later. Hit\
                          \x1b[97mRet\x1b[0m to go back." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void Connect::drawKeysError()
{
    const char* message( "An account key and device key are required for this\
                          connection but they are not set. Hit\
                          \x1b[97mRet\x1b[0m to go back." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void Connect::hideDialogue()
{
    m_dialogue.hide();
}

bool Connect::dial()
{
    String name;
    String number;
    m_phonebook.getDefaultEntry( name, number );
    m_line.dial( number );
    return true;
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
