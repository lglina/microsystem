#include "Audio/MIDIPlayer.h"
#include "Clocks/Clock.h"
#include "EntropySources/EntropySource.h"
#include "InputDevices/InputDevice.h"
#include "Lines/Line.h"
#include "Loggers/Logger.h"
#include "Platforms/Platform.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "UI/Dialogue.h"
#include "UI/Hotkeys.h"
#include "UI/Navigation.h"
#include "UI/NotificationsUI.h"
#include "UI/PlatformUI.h"
#include "UI/PresenceUI.h"
#include "UI/Strategy.h"
#include "UI/VRTime.h"
#include "Utils/LiteStream.h"
#include "World/Compositor.h"
#include "World/Direction.h"
#include "World/User.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "Chat.h"
#include "Collections.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Value.h"
#include "WalkStrategy.h"

using namespace Agape::InputDevices;
using namespace Agape::World;

namespace
{
    const int presenceUpdateInterval( 10000 ); // ms.
    const int notificationsUpdateInterval( 60000 ); // ms.
    const int weatherUpdateInterval( 500 ); // ms.
    const int inactivityInterval( 300000 ); // ms.
    const int reconnectTime( 60000 ); // ms.
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

Walk::Walk( InputDevice& inputDevice,
            Compositor& compositor,
            Chat& chat,
            Metadata& worldMetadata,
            User& worldUser,
            Coordinates& coordinates,
            Hotkeys& hotkeys,
            Navigation& navigation,
            Presence& presence,
            VRTime& vrTime,
            Platform& platform,
            PlatformUI& platformUI,
            NotificationsUI& notificationsUI,
            Timers::Factory& timerFactory,
            Strategy& onboardingStrategy,
            EntropySource& entropySource,
            Clock& clock,
            Line& line,
            Dialogue& dialogue,
            MIDIPlayer& midiPlayer ) :
  m_inputDevice( inputDevice ),
  m_compositor( compositor ),
  m_chat( chat ),
  m_worldMetadata( worldMetadata ),
  m_worldUser( worldUser ),
  m_coordinates( coordinates ),
  m_hotkeys( hotkeys ),
  m_navigation( navigation ),
  m_presence( presence ),
  m_vrTime( vrTime ),
  m_platform( platform ),
  m_platformUI( platformUI ),
  m_notificationsUI( notificationsUI ),
  m_onboardingStrategy( onboardingStrategy ),
  m_entropySource( entropySource ),
  m_clock( clock ),
  m_line( line ),
  m_dialogue( dialogue ),
  m_midiPlayer( midiPlayer ),
  m_state( normal ),
  m_completed( false ),
  m_calling( false ),
  m_holidays( false )
{
    m_presenceUpdateTimer = timerFactory.makeTimer();
    m_notificationsUpdateTimer = timerFactory.makeTimer();
    m_weatherTimer = timerFactory.makeTimer();
    m_inactivityTimer = timerFactory.makeTimer();
    m_reconnectTimer = timerFactory.makeTimer();
}

Walk::~Walk()
{
    delete( m_presenceUpdateTimer );
    delete( m_notificationsUpdateTimer );
    delete( m_weatherTimer );
    delete( m_inactivityTimer );
    delete( m_reconnectTimer );
}

void Walk::enter( const Value& parameters )
{
    m_state = normal;
    m_completed = false;
    m_nextStrategy.clear();
    m_returnParameters = Value();

    setHolidays();
    if( m_holidays )
    {
        // Santa hats ;)
        m_compositor.setCursorVariant( 1 );
    }
    
    if( parameters.hasValue( _coordinates ) )
    {
        m_coordinates = Coordinates::fromValue( parameters[_coordinates] );
        {
        LiteStream stream;
        stream << "Walk: enter: Setting coordinates from parameters:"
               << " worldID: " << m_coordinates.m_worldID
               << " x: " << m_coordinates.m_x
               << " y: " << m_coordinates.m_y;
        LOG_DEBUG( stream.str() );
        }
    }
    else
    {
        m_coordinates.m_worldID = m_worldMetadata.m_worldID;
        m_coordinates.m_x = 0;
        m_coordinates.m_y = 0;
        {
        LiteStream stream;
        stream << "Walk: enter: Setting coordinates to zero for current world:"
               << " worldID: " << m_coordinates.m_worldID
               << " x: " << m_coordinates.m_x
               << " y: " << m_coordinates.m_y;
        LOG_DEBUG( stream.str() );
        }
    }
    m_compositor.setUser( m_worldUser );
    m_compositor.invalidateAllCached(); // Delete cached assets and programs, in case new world has different assets/programs with same names as old world.
    m_compositor.render( m_coordinates );
    if( parameters.hasValue( _row ) && parameters.hasValue( _column ) )
    {
        m_compositor.setPosition( parameters[_row], parameters[_column] );
    }
    else
    {
        m_compositor.setPosition( m_compositor.height() / 2, m_compositor.width() / 2 );
    }
    m_chat.coordinatesChanged();
    m_navigation.draw();
    m_presence.draw();
    m_vrTime.start();
    m_notificationsUI.draw();

    drawHotkeys();

    m_presenceUpdateTimer->reset();
    m_notificationsUpdateTimer->reset();

    createWeather();

    m_inactivityTimer->reset();
    m_chat.setUserActive( true );
    m_notificationsUI.setUserActive( true );

    m_nextStrategy = _message;
    m_calling = true;
}

void Walk::returnTo( const Value& parameters )
{
    if( ( m_nextStrategy == _ansiEditor ) ||
        ( m_nextStrategy == _carlo ) )
    {
        m_vrTime.start();
        m_navigation.draw();
        m_presence.draw();
    }
    else if( m_nextStrategy == _telegram )
    {
        m_notificationsUI.poll(); // To clear unread notification, if any.
    }
    else if( m_nextStrategy == _connect )
    {
        if( (int)parameters[_success] == 1 )
        {
            // Exit and call ourselves again to re-initialise scene.
            m_completed = true;
            m_nextStrategy = _walk;
            return;
        }
        else
        {
            // We'll realise we're still disconnected and prompt the user
            // again, or retry the connection after a time.
            m_state = normal;
        }
    }

    m_calling = false;
    m_nextStrategy.clear();
    m_callingParameters = Value();

    if( (int)parameters[_doTeleport] == 1 )
    {
        LOG_DEBUG( "Walk: returnTo: doTeleport" );
        LOG_DEBUG( parameters.dump() );

        m_midiPlayer.doStop( true ); // true = hard stop.
        m_midiPlayer.doPlay( m_coordinates, _teleport );

        if( (int)parameters[_reenter] == 1 ) // Need to return to EnterWorld so we can go to a new world.
        {
            LOG_DEBUG( "Walk: returnTo: doTeleport: Returning to EnterWorld to switch worlds." );
            m_returnParameters = parameters; // Pass new coords and row,col back to EnterWorld.
            m_completed = true;
        }
        else // Change row/col and/or coordinates in same world.
        {
            LOG_DEBUG( "Walk: returnTo: doTeleport: Changing coordinates " );
            m_coordinates = Coordinates::fromValue( parameters[_coordinates] );
            m_compositor.render( m_coordinates );
            createWeather();
            if( parameters.hasValue( _row ) && parameters.hasValue( _column ) )
            {
                m_compositor.setPosition( parameters[_row], parameters[_column] );
            }
            else
            {
                m_compositor.setPositionToPrevious();
            }
            m_chat.coordinatesChanged();
            m_navigation.draw();
        }
    }
    else if( (int)parameters[_doInvite] == 1 )
    {
        m_nextStrategy = _inviteFriend;
        m_calling = true;
    }

    drawHotkeys();

    m_presenceUpdateTimer->reset();
    m_notificationsUpdateTimer->reset();
    m_weatherTimer->reset();
}

bool Walk::calling( String& strategyName, Value& parameters )
{
    if( m_calling )
    {
        m_chat.normalSize();

        if( ( m_nextStrategy == _ansiEditor ) ||
            ( m_nextStrategy == _carlo ) )
        {
            m_vrTime.stop();
            m_navigation.clear();
            m_presence.clear();
        }

        m_hotkeys.clear();

        strategyName = m_nextStrategy;
        parameters = m_callingParameters;
        
        return true;
    }

    return false;
}

bool Walk::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        m_midiPlayer.doStop( true ); // true = hard stop.

        m_compositor.depart();
        m_chat.stop();
        m_navigation.clear();
        m_presence.clear();
        m_hotkeys.clear();
        m_vrTime.stop();
        m_notificationsUI.clear();

        nextStrategy = m_nextStrategy;
        parameters = m_returnParameters;
        return true;
    }

    return false;
}

void Walk::run()
{
    if( ( m_state == normal ) || ( m_state == offline ) )
    {
        doWalk();

        m_compositor.run();

        if( m_presenceUpdateTimer->ms() > presenceUpdateInterval )
        {
            m_presence.draw();
            m_presenceUpdateTimer->reset();
        }

        if( m_notificationsUpdateTimer->ms() > notificationsUpdateInterval )
        {
            m_notificationsUI.poll();
            m_notificationsUpdateTimer->reset();
        }

        if( !m_completed && !m_calling && m_onboardingStrategy.needsFocus() )
        {
            // Check m_completed so we don't go back into onboarding strategy when
            // it's just returned to us for a teleport or for us to call invite...
            m_nextStrategy = _onboarding;
            m_calling = true;
        }

        if( m_compositor.teleportRequested() )
        {
            m_midiPlayer.doStop( true ); // true = hard stop.
            m_midiPlayer.doPlay( m_coordinates, _teleport );

            Coordinates teleportCoordinates;
            int row( -1 );
            int col( -1 );
            m_compositor.getTeleportDest( teleportCoordinates, row, col );

            if( teleportCoordinates.m_worldID != m_worldMetadata.m_worldID )
            {
                m_returnParameters[_doTeleport] = 1;
                teleportCoordinates.toValue( m_returnParameters[_coordinates] );
                if( ( row != -1 ) && ( col != -1 ) )
                {
                    m_returnParameters[_row] = row;
                    m_returnParameters[_column] = col;
                }
                m_completed = true; // Return to EnterWorld to switch worlds.
            }
            else
            {
                if( teleportCoordinates != m_coordinates )
                {
                    m_coordinates = teleportCoordinates;
                    m_compositor.render( m_coordinates );
                    if( ( row != -1 ) && ( col != -1 ) )
                    {
                        m_compositor.setPosition( row, col );
                    }
                    else
                    {
                        m_compositor.setPositionToPrevious();
                    }
                    m_chat.coordinatesChanged();
                    m_navigation.draw();
                }
                else
                {
                    // Only row and column changed.
                    m_compositor.movePerson( m_worldUser.m_snowflake, row, col );
                    m_compositor.clearTeleportRequested();
                }
            }
        }

        updateWeather();

        if( m_inactivityTimer->ms() >= inactivityInterval )
        {
            m_chat.setUserActive( false );
            m_notificationsUI.setUserActive( false );
        }

        // Check if the connection has dropped.
        if( m_state == normal )
        {
            Line::LineStatus lineStatus( m_line.getLineStatus() );
            if( !lineStatus.m_carrier )
            {
                m_dialogue.show( Dialogue::error );
                m_dialogue.drawTitle( "Connection Lost" );
                m_dialogue.drawMessage( "Your connection has been lost. Hit \x1b[97mRet\x1b[0m to try to reconnect, or \x1b[97mEsc\x1b[0m to continue in offline mode. Auto reconnect will be attempted in 60s." );
                m_state = connLostQuery;
                m_reconnectTimer->reset();
            }
        }
    }
    else if( m_state == connLostQuery )
    {
        // User can confirm reconnect, continue offline, or if user does
        // nothing, try to reconnect after a time.
        while( !m_inputDevice.eof() )
        {
            char c( m_inputDevice.get() );
            if( c == Key::carriageReturn ) continue;
            else if( c == Key::newLine )
            {
                m_dialogue.hide();
                m_state = connLostReconnect;
                m_calling = true;
                m_nextStrategy = _connect;
                m_callingParameters[_nonInteractive] = 1;
            }
            else if( c == Key::escape )
            {
                m_dialogue.hide();
                m_state = offline;
            }
        }

        if( m_reconnectTimer->ms() >= reconnectTime )
        {
            m_dialogue.hide();
            m_state = connLostReconnect;
            m_calling = true;
            m_nextStrategy = _connect;
            m_callingParameters[_nonInteractive] = 1;
        }
    }
}

void Walk::doWalk()
{
    while( !m_inputDevice.eof() )
    {
        bool atEdge( false );
        char c( m_inputDevice.get() );

        if( c == Key::up )
        {
            m_compositor.walk( Direction::up, atEdge );
            if( atEdge )
            {
                ++m_coordinates.m_y;
                m_compositor.render( m_coordinates );
                m_compositor.setPosition( m_compositor.height() - 1, m_compositor.positionCol(), Direction::up );
                m_chat.coordinatesChanged();
                m_navigation.draw();
                createWeather();
            }
        }
        else if( c == Key::down )
        {
            m_compositor.walk( Direction::down, atEdge );
            if( atEdge )
            {
                --m_coordinates.m_y;
                m_compositor.render( m_coordinates );
                m_compositor.setPosition( 0, m_compositor.positionCol(), Direction::down );
                m_chat.coordinatesChanged();
                m_navigation.draw();
                createWeather();
            }
        }
        else if( c == Key::left )
        {
            m_compositor.walk( Direction::left, atEdge );
            if( atEdge )
            {
                --m_coordinates.m_x;
                m_compositor.render( m_coordinates );
                m_compositor.setPosition( m_compositor.positionRow(), m_compositor.width() - 1, Direction::left );
                m_chat.coordinatesChanged();
                m_navigation.draw();
                createWeather();
            }
        }
        else if( c == Key::right )
        {
            m_compositor.walk( Direction::right, atEdge );
            if( atEdge )
            {
                ++m_coordinates.m_x;
                m_compositor.render( m_coordinates );
                m_compositor.setPosition( m_compositor.positionRow(), 0, Direction::right );
                m_chat.coordinatesChanged();
                m_navigation.draw();
                createWeather();
            }
        }
        else if( c == control( 'i' ) )
        {
            m_nextStrategy = _edit;
            m_callingParameters[_mode] = _insert;
            m_calling = true;
        }
        else if( c == control( 'u' ) )
        {
            m_nextStrategy = _edit;
            m_callingParameters[_mode] = _update;
            m_calling = true;
        }
        else if( c == control( 'c' ) )
        {
            m_nextStrategy = _carlo;
            m_calling = true;
        }
        else if( c == control( 'l' ) )
        {
            m_nextStrategy = _linda2;
            m_calling = true;
        }
        else if( c == control( 'a' ) )
        {
            m_compositor.actionMessage();
        }
        else if( c == control( 'm' ) )
        {
            m_nextStrategy = _minimap;
            m_calling = true;
            m_midiPlayer.doStop( true ); // true = hard stop.
        }
        else if( c == control( 'g' ) )
        {
            m_nextStrategy = _telegram;
            m_calling = true;
        }
        else if( c == control( 'd' ) )
        {
            m_nextStrategy = _wood;
            m_completed = true;
        }
        else if( c == control( 'p' ) )
        {
            m_nextStrategy = _teleport;
            m_callingParameters[_row] = m_compositor.positionRow();
            m_callingParameters[_column] = m_compositor.positionCol();
            m_calling = true;
        }
        else if( c == control( 'f' ) )
        {
            m_nextStrategy = _inviteFriend;
            m_calling = true;
        }
        else if( c == control( 'e' ) )
        {
            m_nextStrategy = _ansiEditor;
            m_calling = true;
        }
        else if( c == control( 'x' ) )
        {
            m_chat.toggleMaximise();
        }
        else if( c == control( Key::up ) )
        {
            m_platform.brightnessUp();
        }
        else if( c == control( Key::down ) )
        {
            m_platform.brightnessDown();
        }
        else if( c == control( Key::left ) )
        {
            m_platform.keyboardBrightnessDown();
        }
        else if( c == control( Key::right ) )
        {
            m_platform.keyboardBrightnessUp();
        }
        else if( c == control( Key::backspace ) )
        {
            m_platformUI.showAssetModal( _boss );
        }
        else if( c == '\x1b' )
        {
            m_completed = true;
        }
        else
        {
            m_chat.consumeCharacter( c );
        }

        m_inactivityTimer->reset();
        m_chat.setUserActive( true );
        m_notificationsUI.setUserActive( true );
    }
}

void Walk::drawHotkeys()
{
    m_hotkeys.clear();
    m_hotkeys.show( "\x18\x19\x1B\x1A", "Walk" );
    m_hotkeys.show( "C-A", "ction" );
    m_hotkeys.show( "C-P", "ort" );
    m_hotkeys.show( "C-M", "ap" );
    m_hotkeys.show( "C-D", "Wlds" );
    m_hotkeys.spacer();
    m_hotkeys.show( "C-I", "nsert" );
    m_hotkeys.show( "C-U", "pdate" );
    m_hotkeys.show( "C-E", "dit" );
    m_hotkeys.spacer();
    m_hotkeys.show( "C-G", "ram" );
    m_hotkeys.show( "C-X", "pndChat" );
    m_hotkeys.show( "C-F", "riend" );
    m_hotkeys.spacer();
    m_hotkeys.show( "C-C", "arlo" );
    m_hotkeys.show( "C-L", "inda" );
    m_hotkeys.spacer();
    m_hotkeys.show( "Esc", "Depart" );
}

void Walk::setHolidays()
{
    int day = 0, month = 0, year = 0;
    int hour = 0, minute = 0, second = 0;
    int dayOfWeek = 0;
    Clock::timestampToParts( m_clock.epochS(),
                             day,
                             month,
                             year,
                             hour,
                             minute,
                             second,
                             dayOfWeek );
    m_holidays = ( month == 12 );
}

void Walk::createWeather()
{
    if( m_holidays )
    {
        // SNOW!
        m_weatherTimer->reset();

        for( int i = 0; i < 25; i += 2 )
        {
            unsigned char random;
            m_entropySource.generate( (char*)&random, 1 );

            m_compositor.createSprite( String( 2, 'a' + i ), "snowflake", i, random % 80 );
        }
    }
}

void Walk::updateWeather()
{
    if( m_holidays )
    {
        // SNOW!
        if( m_weatherTimer->ms() > weatherUpdateInterval )
        {
            for( int i = 0; i < 25; i += 2 )
            {
                unsigned char random1;
                m_entropySource.generate( (char*)&random1, 1 );

                unsigned char random2;
                m_entropySource.generate( (char*)&random2, 1 );

                String assetName;
                int row( 0 );
                int col( 0 );
                int height( 0 );
                int width( 0 );
                m_compositor.spriteData( String( 2, 'a' + i ), assetName, row, col, height, width );

                row += random1 % 2;
                if( ( random1 % 2 ) == 1 )
                {
                    col += ( random2 % 3 ) - 1;
                }

                if( row == 25 ) row = 0;
                if( col < 0 ) col = 0;
                if( col == 80 ) col = 79;

                m_compositor.moveSprite( String( 2, 'a' + i ), row, col );
            }

            m_weatherTimer->reset();
        }
    }
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
