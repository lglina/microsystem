#include "InputDevices/InputDevice.h"
#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"  
#include "UI/Forms/Utils/BIP39FormUtil.h"
#include "UI/Forms/Form.h"
#include "UI/Dialogue.h"
#include "World/User.h"
#include "World/WorldMetadata.h"
#include "World/WorldUtilities.h"
#include "WorldLoaders/Factories/WorldLoadersFactory.h"
#include "JoinWorld.h"
#include "Phonebook.h"
#include "StrategyHelper.h"
#include "StringConstants.h"
#include "String.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"
#include "Worldbook.h"

#include <string.h>

using Agape::String;

namespace
{
    const int contentFirstRow( 6 );
    const int contentFirstCol( 8 );
    const int contentMaxWidth( 66 );
    const int contentAttributes( 0x07 );

    const int fieldAttributes( 0x07 );
    
    const int keyFieldWidth( 8 );
    const int keyFieldHeight( 1 );

    const String welcomeTextAssetName( "welcome-text" );
    const String worldsTextAssetName( "worlds-text" );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

JoinWorld::JoinWorld( InputDevice& inputDevice,
                      WindowManager& windowManager,
                      const String& windowName,
                      Worldbook& worldbook,
                      World::Utilities& worldUtilities,
                      WorldLoaders::Factory& worldLoaderFactory,
                      Timers::Factory& timerFactory,
                      Dialogue& dialogue ) :
  m_inputDevice( inputDevice ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_worldbook( worldbook ),
  m_worldUtilities( worldUtilities ),
  m_worldLoaderFactory( worldLoaderFactory ),
  m_timer( timerFactory.makeTimer() ),
  m_dialogue( dialogue ),
  m_state( none ),
  m_completed( false ),
  m_terminal( nullptr ),
  m_currentForm( nullptr )
{
    ::memset( m_key, '\0', 16 );
}

JoinWorld::~JoinWorld()
{
    closeForm();
    delete( m_timer );
}

void JoinWorld::enter( const Value& parameters )
{
    m_completed = false;
    
    m_parameters = parameters;

    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
    {
        if( !parameters.hasValue( _worldKey ) )
        {
            m_terminal = terminalWindow.m_terminal;
            m_state = enterKey;
            drawEnterKey();
        }
        else
        {
            String keyStr = parameters[_worldKey];
            if( keyStr.length() == 16 )
            {
                memcpy( m_key, &keyStr[0], 16 );

                // Key already passed in. Attempt to join immediately.
                drawPending();

                User user;
                Metadata metadata;
                if( joinWorld( user, metadata ) )
                {
                    user.toValue( m_returnParameters[_user] );
                    metadata.toValue( m_returnParameters[_metadata], true ); // true = serialise keys.

                    m_state = success;
                    m_timer->reset();
                    drawSuccess();
                }
                else
                {
                    m_state = error;
                    drawError();
                }
            }
            else
            {
                m_returnParameters[_success] = 0;
                m_completed = true; // Uh oh!
            }
        }
    }
    else
    {
        m_returnParameters[_success] = 0;
        m_completed = true; // Uh oh!
    }
}

void JoinWorld::returnTo( const Value& parameters )
{
}

bool JoinWorld::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool JoinWorld::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        closeForm();
        parameters = m_returnParameters;
        return true;
    }

    return false;
}

void JoinWorld::run()
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
                    if( getKey( m_key ) )
                    {
                        drawPending();

                        User user;
                        Metadata metadata;
                        if( joinWorld( user, metadata ) )
                        {
                            user.toValue( m_returnParameters[_user] );
                            metadata.toValue( m_returnParameters[_metadata], true ); // true = serialise keys.

                            m_state = success;
                            m_timer->reset();
                            drawSuccess();
                        }
                        else
                        {
                            m_state = error;
                            drawError();
                        }
                    }
                    else
                    {
                        m_state = error;
                        drawKeyError();
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
                if( c == '\x08' )
                {
                    // If field empty, move to previous field and execute backspace.
                    if( m_currentForm->currentField().m_contents.length() == 0 )
                    {
                        m_currentForm->prevField();
                    }
                }

                m_currentForm->consumeChar( c );
                
                if( c != '\x08' )
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

    if( m_state == success )
    {
        if( m_timer->ms() >= 2000 )
        {
            hideDialogue();
            m_returnParameters[_success] = 1;
            m_completed = true;
        }
    }
}

void JoinWorld::drawBackground()
{
    if( m_terminal != nullptr )
    {
        String textAssetName;
        if( m_parameters[_title] == String( _welcome ) )
        {
            textAssetName = welcomeTextAssetName;
        }
        else if( m_parameters[_title] == String( _worlds ) )
        {
            textAssetName = worldsTextAssetName;
        }

        Helper::drawMenuBackground( m_terminal, textAssetName );
    }
}

void JoinWorld::drawEnterKey()
{
    drawBackground();

    const char* message( "OK! You'll need to enter the twelve word key for the world below.\
                          You only need to enter the first few letters of each word - we'll fill in the rest.\
                          When you're done, hit \x1b[97mRet\x1b[0m. If you make a mistake, hit \x1b[97mBs\x1b[0m.\
                          You can also hit \x1b[97mTab\x1b[0m or \x1b[97mSh-Tab\x1b[0m to switch fields." );
    
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
    Forms::Utils::BIP39::createKeyForm( fields,
                                        contentFirstRow + 5,
                                        contentFirstCol,
                                        fieldAttributes );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager, m_windowName, String(), fields, Forms::Title() );
    m_currentForm->openUI();

    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->draw();
    }
}

void JoinWorld::drawPending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Please Wait" );
    m_dialogue.drawMessage( "Joining world" );
}

void JoinWorld::drawSuccess()
{
    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( "Joined successfully" );
}

void JoinWorld::drawError()
{
    const char* message( "We were unable to join you to that\
                          world. Please check with the owner\
                          of the world that you have the\
                          correct key.\
                          Hit \x1b[97mRet\x1b[0m to go back." );
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void JoinWorld::drawKeyError()
{
    const char* message( "The key you entered was invalid. Only use words\
                          from the key word list, and check your spelling.\
                          Hit \x1b[97mRet\x1b[0m to go back." );
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void JoinWorld::hideDialogue()
{
    m_dialogue.hide();
}

void JoinWorld::closeForm()
{
    if( m_currentForm != nullptr )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

bool JoinWorld::getKey( char* key )
{
    if( m_currentForm != nullptr )
    {
        return Forms::Utils::BIP39::getKey( m_currentForm, key );
    }

    return false;
}

bool JoinWorld::joinWorld( User& user, Metadata& plainMetadata )
{
    User userParameters( User::fromValue( m_parameters[_user] ) );

    m_worldUtilities.generateJoinMetadata( m_key,
                                           m_parameters[_accountSubKey],
                                           userParameters.m_name,
                                           userParameters.m_glyph,
                                           userParameters.m_attributes,
                                           plainMetadata,
                                           user );
    
    World::Metadata encryptedMetadata;
    m_worldUtilities.encryptMetadata( m_key, plainMetadata, encryptedMetadata );

    WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );
    String reason; // FIXME: Show failure reason to user?
    bool success( worldLoader->join( encryptedMetadata, reason ) );
    if( success )
    {
        m_worldUtilities.decryptMetadata( m_key, encryptedMetadata, plainMetadata );
        // Add world key to the metadata as returned from the server, so the
        // world key can be returned from this strategy to be saved in
        // the worldbook.
        plainMetadata.m_worldKey = String( m_key, 16 );
    }

    delete( worldLoader );
    return success;
}

void JoinWorld::autocompleteKeyword()
{
    if( m_currentForm != nullptr )
    {
        Forms::Utils::BIP39::tryAutoComplete( m_currentForm );
    }
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
