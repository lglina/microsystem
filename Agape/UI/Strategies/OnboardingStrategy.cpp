#include "Assets/ANSIFile.h"
#include "AssetLoaders/BakedAssetLoader.h"
#include "Clocks/Clock.h"
#include "EnterKeyStrategy.h"
#include "InputDevices/InputDevice.h"
#include "Utils/Snowflake.h"
#include "World/Teleport.h"
#include "World/User.h"
#include "World/WorldMetadata.h"
#include "World/WorldUtilities.h"
#include "ConfigurationStore.h"
#include "OnboardingStrategy.h"
#include "Phonebook.h"
#include "StrategyHelper.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"
#include "Worldbook.h"

using Agape::String;

using namespace Agape::InputDevices;
using namespace Agape::World;

namespace
{
    const int contentFirstRow( 6 );
    const int contentFirstCol( 8 );
    const int contentMaxWidth( 66 );
    const int contentAttributes( 0x07 );

    const int dialogueFirstRow( 2 );
    const int dialogueFirstCol( 1 );
    const int dialogueContentMaxWidth( 51 );

    const int teleportDemoDelay( 30 ); // seconds.
    const int inviteFriendDelay( 300 ); // seconds.

    const int titleRow( 0 );
    const int titleAttributes( 0x9F );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

Onboarding::Onboarding( InputDevice& inputDevice,
                        WindowManager& windowManager,
                        Phonebook& phonebook,
                        Worldbook& worldbook,
                        const Metadata& worldMetadata,
                        const User& worldUser,
                        ConfigurationStore& configurationStore,
                        Clock& clock,
                        Snowflake& snowflake,
                        const String& mainWindowName,
                        const String& dialogueWindowName,
                        const String& defaultPhoneName,
                        const String& defaultPhoneNumber,
                        const String& learningWorldID,
                        const char* learningWorldKey ) :
  m_inputDevice( inputDevice ),
  m_windowManager( windowManager ),
  m_phonebook( phonebook ),
  m_worldbook( worldbook ),
  m_worldMetadata( worldMetadata ),
  m_worldUser( worldUser ),
  m_configurationStore( configurationStore ),
  m_clock( clock ),
  m_snowflake( snowflake ),
  m_mainWindowName( mainWindowName ),
  m_dialogueWindowName( dialogueWindowName ),
  m_defaultPhoneName( defaultPhoneName ),
  m_defaultPhoneNumber( defaultPhoneNumber ),
  m_learningWorldID( learningWorldID ),
  m_learningWorldKey( learningWorldKey ),
  m_state( none ),
  m_stateTime( clock.epochS() ),
  m_calling( false ),
  m_completed( false ),
  m_needTeleport( false ),
  m_mainTerminal( nullptr ),
  m_dialogueTerminal( nullptr )
{
}

void Onboarding::enter( const Value& parameters )
{
    m_completed = false;
    m_nextStrategy.clear();
    m_returnParameters = Value();

    if( m_state == none )
    {
        // Called from MenuStrategy on reboot.
        // Look for saved state.
        if( m_configurationStore.hasKey( _oobe ) )
        {
            Value& oobeValue( m_configurationStore.get( _oobe ) );
            if( oobeValue.hasValue( _state ) )
            {
                String state = oobeValue[_state];
                if( state == _teleportDemo )
                {
                    // Offer to teleport to learning world is next. Return
                    // and wait to be called from WalkStrategy based on
                    // needsFocus() on time trigger.
                    m_state = teleportDemo;
                    m_completed = true;
                }
                else if( state == _inviteFriend )
                {
                    // Offer to invite friend is next. Return and wait to be
                    // called from WalkStrategy based on needsFocus() on
                    // time trigger.
                    m_state = inviteFriend;
                    m_completed = true;
                }
                else if( state == _completed )
                {
                    // Onboarding already completed.
                    m_state = completed;
                    m_completed = true;
                }
            }
        }
        
        WindowManager::TerminalWindow terminalWindow;
        if( m_windowManager.getTerminalWindow( m_mainWindowName, terminalWindow ) )
        {
            m_mainTerminal = terminalWindow.m_terminal;
        }
        else
        {
            m_completed = true; // Uh oh!
        }

        if( m_windowManager.getTerminalWindow( m_dialogueWindowName, terminalWindow ) )
        {
            m_dialogueTerminal = terminalWindow.m_terminal;
        }
        else
        {
            m_completed = true; // Uh oh!
        }

        if( !m_completed )
        {
            // No saved state - go to welcome.
            drawWelcome();
            m_state = welcome;
        }
    }
    else if( m_state == teleportDemo )
    {
        // Called due to needsFocus().
        drawTeleportDemo();
    }
    else if( m_state == teleportDone )
    {
        // Called due to needsFocus().
        drawTeleportDone();
    }
    else if( m_state == inviteFriend )
    {
        // Called due to needsFocus().
        drawInviteFriend();
    }
    else
    {
        // ???
        m_completed = true;
    }
}

void Onboarding::returnTo( const Value& parameters )
{
    m_calling = false;
    m_nextStrategy.clear();
    m_callingParameters = Value();

    switch( m_state )
    {
    case enterAccountKey: // <- Returning from
        if( (int)parameters[_success] == 1 )
        {
            if( !m_configurationStore.hasKey( _deviceAuthKey ) )
            {
                m_state = enterDeviceKey;
                m_nextStrategy = "enterKey";
                m_callingParameters[_title] = _welcome;
                m_callingParameters[_keyType] = EnterKey::kDevice;
                m_calling = true;
            }
            else
            {
                m_state = connectWifi;
                m_nextStrategy = "connectWiFi";
                m_callingParameters[_title] = _welcome;
                m_callingParameters[_mode] = _add;
                m_calling = true;
            }
        }
        else
        {
            // Aborted
            drawWelcome();
            m_state = welcome;
        }
        break;
    case enterDeviceKey: // <- Returning from
        if( (int)parameters[_success] == 1 )
        {
            m_state = connectWifi;
            m_nextStrategy = "connectWiFi";
            m_callingParameters[_title] = _welcome;
            m_callingParameters[_mode] = _add;
            m_calling = true;
        }
        else
        {
            // Aborted
            drawWelcome();
            m_state = welcome;
        }
        break;
    case connectWifi: // <- Returning from
        if( (int)parameters[_success] == 1 )
        {
            m_phonebook.add( m_defaultPhoneName, m_defaultPhoneNumber );
            m_state = connect;
            m_nextStrategy = "connect";
            m_calling = true;
        }
        else
        {
            // Aborted
            drawWelcome();
            m_state = welcome;
        }
        break;
    case connect: // <- Returning from
        if( (int)parameters[_success] == 1 )
        {
            m_state = createOrJoinWorld;
            drawCreateOrJoinWorld();
        } // Else call again!
        else
        {
            m_nextStrategy = "connect";
            m_calling = true;
        }
        break;
    case chooseAvatarCreate: // <- Returning from
        m_state = createWorld;
        m_nextStrategy = "createWorld";
        m_callingParameters[_title] = _welcome;
        m_callingParameters[_user] = parameters;
        if( m_configurationStore.hasKey( _accountSubKey ) )
        {
            m_callingParameters[_accountSubKey] = m_configurationStore.get( _accountSubKey );
        }
        m_calling = true;
        break;
    case chooseAvatarJoin: // <- Returning from
        m_state = joinWorld;
        m_nextStrategy = "joinWorld";
        m_callingParameters[_title] = _welcome;
        m_callingParameters[_user] = parameters;
        if( m_configurationStore.hasKey( _accountSubKey ) )
        {
            m_callingParameters[_accountSubKey] = m_configurationStore.get( _accountSubKey );
        }
        m_calling = true;
        break;
    case createWorld: // <- Returning from
    case joinWorld:
        if( (int)parameters[_success] == 1 )
        {
            Metadata metadata( Metadata::fromValue( parameters[_metadata], true ) ); // true = deserialise keys.
            User user( User::fromValue( parameters[_user] ) );

            m_worldbook.add( metadata, true ); // true = make default.
            m_worldbook.setUserForWorld( metadata.m_worldID, user );

            m_state = teleportDemo;
            m_completed = true;
        }
        else
        {
            m_state = createOrJoinWorld;
            drawCreateOrJoinWorld();
        }
        break;
    case joinLearning: // <- Returning from
        if( (int)parameters[_success] == 1 )
        {
            // Add learning world to world book.
            Metadata metadata( Metadata::fromValue( parameters[_metadata], true ) ); // true = deserialise keys.
            User user( User::fromValue( parameters[_user] ) );

            m_worldbook.add( metadata, false ); // false = don't make default.
            m_worldbook.setUserForWorld( metadata.m_worldID, user );

            // Add teleport to home world.
            Teleport teleport;
            teleport.m_snowflake = m_snowflake.generate();
            teleport.m_name = "My home";
            teleport.m_row = 12;
            teleport.m_col = 40;
            teleport.m_coordinates = Coordinates( m_worldMetadata.m_worldID );
            teleport.toValue( m_callingParameters[_teleport] );
            m_callingParameters[_doTeleport] = 0;
            m_callingParameters[_home] = 1;

            m_state = addTeleportHome;
            m_nextStrategy = "teleport";
            m_calling = true;
        }
        else
        {
            m_completed = true; // !! Abandon and will try again on restart?
        }
        break;
    case addTeleportHome: // <- Returning from
        if( (int)parameters[_success] == 1 )
        {
            // Add teleport to learning world.
            Teleport teleport;
            teleport.m_snowflake = m_snowflake.generate();
            teleport.m_name = "Learning home";
            teleport.m_row = 12;
            teleport.m_col = 40;
            teleport.m_coordinates = Coordinates( m_learningWorldID );
            teleport.toValue( m_callingParameters[_teleport] );
            m_callingParameters[_doTeleport] = m_needTeleport ? 1 : 0;
            m_callingParameters[_home] = 0;

            m_state = addTeleportDemo;
            m_nextStrategy = "teleport";
            m_calling = true;
        }
        else
        {
            m_completed = true; // !! Abandon and will try again on restart?
        }
        break;
    case addTeleportDemo: // <- Returning from
        if( (int)parameters[_success] == 1 )
        {
            if( m_needTeleport )
            {
                m_state = teleportDone;
                m_returnParameters = parameters;
                m_completed = true;
            }
            else
            {
                m_state = teleportDeclined;
                drawTeleportDeclined();
            }
        }
        else
        {
            m_completed = true; // !! Abandon and will try again on restart?
        }
        break;
    default:
        break;
    }
}

bool Onboarding::calling( String& strategyName, Value& parameters )
{
    if( m_calling )
    {
        strategyName = m_nextStrategy;
        parameters = m_callingParameters;
        return true;
    }

    return false;
}

bool Onboarding::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        nextStrategy = m_nextStrategy;
        parameters = m_returnParameters;

        if( m_state == teleportDemo )
        {
            Value& oobeValue( m_configurationStore.get( _oobe ) );
            oobeValue[_state] = _teleportDemo;
            m_configurationStore.save();

            m_stateTime = m_clock.epochS(); // Start wait for asking to teleport.
        }
        else if( m_state == inviteFriend )
        {
            Value& oobeValue( m_configurationStore.get( _oobe ) );
            oobeValue[_state] = _inviteFriend;
            m_configurationStore.save();

            m_stateTime = m_clock.epochS(); // Start wait for asking to invite.
        }
        else if( m_state == completed )
        {
            Value& oobeValue( m_configurationStore.get( _oobe ) );
            oobeValue[_state] = _completed;
            m_configurationStore.save();
        }

        return true;
    }

    return false;
}

bool Onboarding::needsFocus()
{
    return( ( m_state == teleportDemo && ( ( m_clock.epochS() - m_stateTime ) >= teleportDemoDelay ) ) ||
            ( m_state == inviteFriend && ( m_worldMetadata.m_worldID != m_learningWorldID ) && ( ( m_clock.epochS() - m_stateTime ) >= inviteFriendDelay ) ) ||
            ( m_state == teleportDone ) );
}

void Onboarding::run()
{
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }

        if( c == control( 'z' ) )
        {
            hideDialogue();
            m_state = completed;
            m_completed = true;
            break;
        }

        switch( m_state )
        {
        case welcome:
            if( c == '\n' )
            {
                if( !m_configurationStore.hasKey( _accountAuthKey ) )
                {
                    m_state = enterAccountKey;
                    m_nextStrategy = "enterKey";
                    m_callingParameters[_title] = _welcome;
                    m_callingParameters[_keyType] = EnterKey::kAccount;
                    m_calling = true;
                }
                else if( !m_configurationStore.hasKey( _deviceAuthKey ) )
                {
                    m_state = enterDeviceKey;
                    m_nextStrategy = "enterKey";
                    m_callingParameters[_title] = _welcome;
                    m_callingParameters[_keyType] = EnterKey::kDevice;
                    m_calling = true;
                }
                else
                {
                    m_state = connectWifi;
                    m_nextStrategy = "connectWiFi";
                    m_callingParameters[_title] = _welcome;
                    m_callingParameters[_mode] = _add;
                    m_calling = true;
                }
            }
            break;
        case createOrJoinWorld:
            if( c == 'c' )
            {
                m_state = chooseAvatarCreate;
                m_nextStrategy = "chooseAvatar";
                m_callingParameters[_title] = _welcome;
                m_calling = true;
            }
            else if( c == 'j' )
            {
                m_state = chooseAvatarJoin;
                m_nextStrategy = "chooseAvatar";
                m_callingParameters[_title] = _welcome;
                m_calling = true;
            }
            break;
        case teleportDemo:
            if( c == 'y' )
            {
                m_state = joinLearning;
                m_needTeleport = true;
                m_nextStrategy = "joinWorld";
                m_callingParameters[_worldKey] = String( m_learningWorldKey, 16 );
                m_worldUser.toValue( m_callingParameters[_user] );
                if( m_configurationStore.hasKey( _accountSubKey ) )
                {
                    m_callingParameters[_accountSubKey] = m_configurationStore.get( _accountSubKey );
                }
                m_calling = true;
            }
            else if( c == 'n' )
            {
                m_state = joinLearning;
                m_needTeleport = false;
                m_nextStrategy = "joinWorld";
                m_callingParameters[_worldKey] = String( m_learningWorldKey, 16 );
                m_worldUser.toValue( m_callingParameters[_user] );
                if( m_configurationStore.hasKey( _accountSubKey ) )
                {
                    m_callingParameters[_accountSubKey] = m_configurationStore.get( _accountSubKey );
                }
                m_calling = true;
            }
            break;
        case inviteFriend:
            if( c == 'y' )
            {
                m_returnParameters[_doInvite] = 1;

                hideDialogue();
                m_state = completed;
                m_completed = true;
            }
            else if( c == 'n' )
            {
                drawInviteDeclined();
                m_state = inviteDeclined;
            }
            break;
        case teleportDone:
        case teleportDeclined:
            if( c == '\n' )
            {
                hideDialogue();
                m_state = inviteFriend;
                m_completed = true;
            }
            break;
        case inviteDeclined:
            if( c == '\n' )
            {
                hideDialogue();
                m_state = completed;
                m_completed = true;
            }
            break;
        default:
            break;
        }
    }
}

void Onboarding::drawMainBackground()
{
    Helper::drawMenuBackground( m_mainTerminal, "welcome-text" );
}

void Onboarding::drawDialogueBackground( const String& title )
{
    AssetLoaders::Baked assetLoader( Coordinates(), "large-dialogue" );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        m_dialogueTerminal->clearScreen();
        m_dialogueTerminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
    }

    m_dialogueTerminal->printFormatted( title,
                                        titleRow,
                                        0, // Col.
                                        1, // Lines.
                                        dialogueContentMaxWidth,
                                        Terminal::hCentre,
                                        Terminal::noVCentre,
                                        titleAttributes,
                                        Terminal::preserveBackground );
}

void Onboarding::drawWelcome()
{
    drawMainBackground();

    const char* message1( "Welcome to the Micro System, junior computer engineer!\
                           Congratulations on getting your computer up and running. We hope\
                           you're excited about getting started exploring, building and\
                           chatting \x02" );
    const char* message2( "First, though, we need to get you connected to the Internet,\
                           find a world to connect to, and have you choose a name and character." );
    const char* message3( "Hit the \x1b[97mRet\x1b[0m key to continue." );

    m_mainTerminal->printFormatted( message1,
                                    contentFirstRow,
                                    contentFirstCol,
                                    Terminal::noMaxHeight,
                                    contentMaxWidth,
                                    Terminal::noHCentre,
                                    Terminal::noVCentre,
                                    contentAttributes,
                                    Terminal::preserveBackground );

    m_mainTerminal->printFormatted( message2,
                                    contentFirstRow + 5,
                                    contentFirstCol,
                                    Terminal::noMaxHeight,
                                    contentMaxWidth,
                                    Terminal::noHCentre,
                                    Terminal::noVCentre,
                                    contentAttributes,
                                    Terminal::preserveBackground );
    
    m_mainTerminal->printFormatted( message3,
                                    contentFirstRow + 8,
                                    contentFirstCol,
                                    Terminal::noMaxHeight,
                                    contentMaxWidth,
                                    Terminal::noHCentre,
                                    Terminal::noVCentre,
                                    contentAttributes,
                                    Terminal::preserveBackground );
}

void Onboarding::drawCreateOrJoinWorld()
{
    drawMainBackground();

    const char* message1( "Now that you're connected, do you want to create a new world, or join an\
                           existing one? If you want to join an existing world, you'll need\
                           the owner of the world to give you the key." );
    const char* message2( "Hit \x1b[97mC\x1b[0m to create a new world." );
    const char* message3( "Hit \x1b[97mJ\x1b[0m to join an existing world." );

    m_mainTerminal->printFormatted( message1,
                                    contentFirstRow,
                                    contentFirstCol,
                                    Terminal::noMaxHeight,
                                    contentMaxWidth,
                                    Terminal::noHCentre,
                                    Terminal::noVCentre,
                                    contentAttributes,
                                    Terminal::preserveBackground );

    m_mainTerminal->printFormatted( message2,
                                    contentFirstRow + 4,
                                    contentFirstCol,
                                    Terminal::noMaxHeight,
                                    contentMaxWidth,
                                    Terminal::noHCentre,
                                    Terminal::noVCentre,
                                    contentAttributes,
                                    Terminal::preserveBackground );
    
    m_mainTerminal->printFormatted( message3,
                                    contentFirstRow + 6,
                                    contentFirstCol,
                                    Terminal::noMaxHeight,
                                    contentMaxWidth,
                                    Terminal::noHCentre,
                                    Terminal::noVCentre,
                                    contentAttributes,
                                    Terminal::preserveBackground );
}

void Onboarding::drawTeleportDemo()
{
    drawDialogueBackground( _Welcome );

    const char* message1( "Welcome to your world! You can start building here now, but\
                           perhaps you'd like to take a look at the 'Micro School' world first,\
                           to learn how to build and see some examples." );
                           
    const char* message2( "Teleport to Micro School now?" );

    const char* message3( "Hit \x1b[97mY\x1b[0m for Yes, or \x1b[97mN\x1b[0m for No." );

    m_dialogueTerminal->printFormatted( message1,
                                        dialogueFirstRow,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );

    m_dialogueTerminal->printFormatted( message2,
                                        dialogueFirstRow + 5,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );

    m_dialogueTerminal->printFormatted( message3,
                                        dialogueFirstRow + 7,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );
    
    showDialogue();
}

void Onboarding::drawTeleportDone()
{
    drawDialogueBackground( _Welcome );

    const char* message1( "Welcome to Micro School! Feel free to take a look around." );

    const char* message2( "If you walk down you'll find an object yard containing all the different things\
                           you can build. Up you'll find building and coding help, and to the right you'll\
                           find some example games - you can copy the code from these into your own world\
                           when you return." );

    const char* message3( "When you're done and want to go back to your own world, hit \x1b[97mC-P\x1b[0m\
                           to open the Teleports window, then select \"My home\" and hit \x1b[97mRet\x1b[0m." );
    
    const char* message4( "Hit \x1b[97mRet\x1b[0m to continue." );

    m_dialogueTerminal->printFormatted( message1,
                                        dialogueFirstRow,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );

    m_dialogueTerminal->printFormatted( message2,
                                        dialogueFirstRow + 3,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );

    m_dialogueTerminal->printFormatted( message3,
                                        dialogueFirstRow + 10,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );
    
    m_dialogueTerminal->printFormatted( message4,
                                        dialogueFirstRow + 14,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );
    
    showDialogue();
}

void Onboarding::drawTeleportDeclined()
{
    drawDialogueBackground( _Welcome );

    const char* message1( "OK! We've added Micro School to your list of teleports so you can go there later\
                           if you choose. To teleport there, hit \x1b[97mC-P\x1b[0m to open the Teleports window,\
                           select \"Learning home\", then hit \x1b[97mRet\x1b[0m." );

    const char* message2( "Hit \x1b[97mRet\x1b[0m to continue." );

    m_dialogueTerminal->printFormatted( message1,
                                        dialogueFirstRow,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );

    m_dialogueTerminal->printFormatted( message2,
                                        dialogueFirstRow + 3,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );

    showDialogue();
}

void Onboarding::drawInviteFriend()
{
    drawDialogueBackground( _Invite_A_Friend );

    const char* message1( "The Micro System is better with friends. Invite your friends to come and play with you now!\
                           They'll receive an email with a link to join you with their web browser." );

    const char* message2( "Invite a friend now?" );

    const char* message3( "Hit \x1b[97mY\x1b[0m for Yes, or \x1b[97mN\x1b[0m for No." );

    m_dialogueTerminal->printFormatted( message1,
                                        dialogueFirstRow,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );

    m_dialogueTerminal->printFormatted( message2,
                                        dialogueFirstRow + 5,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );

    m_dialogueTerminal->printFormatted( message3,
                                        dialogueFirstRow + 7,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );
    
    showDialogue();
}

void Onboarding::drawInviteDeclined()
{
    drawDialogueBackground( _Invite_A_Friend );

    const char* message1( "OK! If you want to invite friends later, just hit \x1b[97mC-F\x1b[0m." );

    const char* message2( "Hit \x1b[97mRet\x1b[0m to continue." );

    m_dialogueTerminal->printFormatted( message1,
                                        dialogueFirstRow,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );

    m_dialogueTerminal->printFormatted( message2,
                                        dialogueFirstRow + 3,
                                        dialogueFirstCol,
                                        Terminal::noMaxHeight,
                                        dialogueContentMaxWidth,
                                        Terminal::noHCentre,
                                        Terminal::noVCentre,
                                        contentAttributes,
                                        Terminal::preserveBackground );
}

void Onboarding::showDialogue()
{
    m_windowManager.setTerminalWindowVisible( m_dialogueWindowName, true );
}

void Onboarding::hideDialogue()
{
    m_windowManager.setTerminalWindowVisible( m_dialogueWindowName, false );
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
