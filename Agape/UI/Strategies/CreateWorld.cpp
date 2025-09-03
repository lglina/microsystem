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
#include "CreateWorld.h"
#include "Phonebook.h"
#include "StrategyHelper.h"
#include "StringConstants.h"
#include "String.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"

#include <string.h>

using Agape::String;

namespace
{
    const int contentFirstRow( 6 );
    const int contentFirstCol( 8 );
    const int contentMaxWidth( 66 );
    const int contentAttributes( 0x07 );

    const int fieldAttributes( 0x07 );
    const int textWidth( 50 );
    const int textHeight( 1 );
    
    const int keyWordAttributes( 0x0F );

    const String welcomeTextAssetName( "welcome-text" );
    const String worldsTextAssetName( "worlds-text" );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

CreateWorld::CreateWorld( InputDevice& inputDevice,
                      WindowManager& windowManager,
                      const String& windowName,
                      World::Utilities& worldUtilities,
                      WorldLoaders::Factory& worldLoaderFactory,
                      Timers::Factory& timerFactory,
                      Dialogue& dialogue ) :
  m_inputDevice( inputDevice ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_worldUtilities( worldUtilities ),
  m_worldLoaderFactory( worldLoaderFactory ),
  m_timer( timerFactory.makeTimer() ),
  m_dialogue( dialogue ),
  m_state( none ),
  m_completed( false ),
  m_terminal( nullptr ),
  m_currentForm( nullptr )
{
}

CreateWorld::~CreateWorld()
{
    closeForm();
    delete( m_timer );
}

void CreateWorld::enter( const Value& parameters )
{
    m_completed = false;
    
    m_parameters = parameters;

    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
        m_state = enterName;
        drawEnterName();
    }
    else
    {
        m_returnParameters[_success] = 0;
        m_completed = true; // Uh oh!
    }
}

void CreateWorld::returnTo( const Value& parameters )
{
}

bool CreateWorld::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool CreateWorld::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        closeForm();
        parameters = m_returnParameters;
        return true;
    }

    return false;
}

void CreateWorld::run()
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
        case enterName:
            if( c == '\n' )
            {
                if( m_currentForm != nullptr )
                {
                    m_name = m_currentForm->getFieldContents( _name );

                    m_worldUtilities.generateWorldKey( m_key );
                    m_state = generateKey;
                    drawGenerateKey( m_key );
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
            }
            break;
        case generateKey:
            if( c == '\n' )
            {
                drawPending();

                User user;
                Metadata metadata;
                if( createWorld( user, metadata ) )
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
            else if( c == 'r' || c == 'R' )
            {
                m_worldUtilities.generateWorldKey( m_key );
                drawGenerateKey( m_key ); // Regenerate.
            }
            else if( c == 'x' )
            {
                m_state = enterKey;
                drawEnterKey();
            }
            else if( c == '\x1b' )
            {
                m_state = enterName;
                drawEnterName();
            }
            break;
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
                        if( createWorld( user, metadata ) )
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
                m_state = enterName;
                drawEnterName();
            }
            else if( m_currentForm != nullptr )
            {
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

void CreateWorld::drawBackground()
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

void CreateWorld::drawEnterName()
{
    drawBackground();

    const char* message( "What do you want to call your new world? Type a name\
                          below, then hit \x1b[97mRet\x1b[0m." );
    m_terminal->printFormatted( message,
                                contentFirstRow,
                                contentFirstCol,
                                2, // Lines max.
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );
    
    Vector< Forms::Field > fields;
    fields.push_back( Forms::Field( _name,
                                    textWidth,
                                    textHeight,
                                    contentFirstRow + 3,
                                    contentFirstCol,
                                    fieldAttributes ) );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager, m_windowName, String(), fields, Forms::Title() );
    m_currentForm->openUI();

    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->draw();
    }
}

void CreateWorld::drawGenerateKey( const char* key )
{
    closeForm();

    drawBackground();

    const char* message( "Here's the key for your new world. \x1b[97mPlease write it down now\x1b[0m.\
                          You'll need to give it to your friends so they can join you.\
                          \x1b[97mPlease keep it secret\x1b[0m - anyone who has it can join your world." );
    m_terminal->printFormatted( message,
                                contentFirstRow,
                                contentFirstCol,
                                3, // Lines max.
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );

    Forms::Utils::BIP39::showKey( key,
                                  m_terminal,
                                  contentFirstRow,
                                  contentFirstCol,
                                  keyWordAttributes );

    m_terminal->printFormatted( "Hit \x1b[97mRet\x1b[0m to continue.",
                                contentFirstRow + 8,
                                contentFirstCol,
                                1, // Lines max.
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );

    const char* message2( "Note: Keys are generated randomly from a list of dictionary words\
                           and have no meaning. We apologise, however, if you feel your key is\
                           inappropriate or offensive. Hit \x1b[97mR\x1b[0m to create a new one." );
    m_terminal->printFormatted( message2,
                                contentFirstRow + 10,
                                contentFirstCol,
                                3, // Lines max.
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );
}

void CreateWorld::drawEnterKey()
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

void CreateWorld::drawPending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Please Wait" );
    m_dialogue.drawMessage( "Creating world" );
}

void CreateWorld::drawSuccess()
{
    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( "Created successfully" );
}

void CreateWorld::drawError()
{
    const char* message( "We were unable to create a world for\
                          you. Please try again. If the problem\
                          continues, please contact support.\
                          Hit \x1b[97mRet\x1b[0m to go back." );
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void CreateWorld::drawKeyError()
{
    const char* message( "The key you entered was invalid. Only use words\
                          from the key word list, and check your spelling.\
                          Hit \x1b[97mRet\x1b[0m to go back." );
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void CreateWorld::hideDialogue()
{
    m_dialogue.hide();
}

void CreateWorld::closeForm()
{
    if( m_currentForm != nullptr )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

bool CreateWorld::getKey( char* key )
{
    if( m_currentForm != nullptr )
    {
        return Forms::Utils::BIP39::getKey( m_currentForm, key );
    }

    return false;
}

bool CreateWorld::createWorld( User& user, Metadata& plainMetadata )
{
    User userParameters( User::fromValue( m_parameters[_user] ) );

    m_worldUtilities.generateCreateMetadata( m_key,
                                             m_parameters[_accountSubKey],
                                             m_name,
                                             userParameters.m_name,
                                             userParameters.m_glyph,
                                             userParameters.m_attributes,
                                             plainMetadata,
                                             user );

    World::Metadata encryptedMetadata;
    m_worldUtilities.encryptMetadata( m_key, plainMetadata, encryptedMetadata );

    WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );
    String reason; // FIXME: Show failure reason to user?
    bool success( worldLoader->create( encryptedMetadata, reason ) );

    delete( worldLoader );
    return success;
}

void CreateWorld::autocompleteKeyword()
{
    if( m_currentForm != nullptr )
    {
        Forms::Utils::BIP39::tryAutoComplete( m_currentForm );
    }
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
