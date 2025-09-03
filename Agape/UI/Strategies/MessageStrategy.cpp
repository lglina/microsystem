#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/AssetLoader.h"
#include "Assets/ANSIFile.h"
#include "InputDevices/InputDevice.h"
#include "Timers/Factories/TimerFactory.h"
#include "Utils/LiteStream.h"
#include "World/WorldCoordinates.h"
#include "ANSITerminal.h"
#include "ConfigurationStore.h"
#include "MessageStrategy.h"
#include "String.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"

namespace
{
    const int timeout( 20000 ); // ms
} // Anonymous namespace

namespace Agape
{

using namespace World;

namespace UI
{

namespace Strategies
{

Message::Message( WindowManager& windowManager,
                  const String& windowName,
                  InputDevice& inputDevice,
                  Timers::Factory& timerFactory,
                  AssetLoaders::Factory& assetLoaderFactory,
                  Coordinates& coordinates,
                  ConfigurationStore& configurationStore ) :
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_inputDevice( inputDevice ),
  m_timer( timerFactory.makeTimer() ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_coordinates( coordinates ),
  m_configurationStore( configurationStore ),
  m_completed( false ),
  m_lastSecond( 0 ),
  m_terminal( nullptr )
{
}

Message::~Message()
{
    delete( m_timer );
}

void Message::enter( const Value& parameters )
{
    m_completed = true;

    // Never show server or world MOTD pop-ups if onboarding not
    // completed, to minimise visual distractions for new users.
    if( m_configurationStore.hasKey( _oobe ) )
    {
        Value& oobeValue( m_configurationStore.get( _oobe ) );
        if( oobeValue.hasValue( _state ) )
        {
            String state = oobeValue[_state];
            if( state == _completed ) // <-- If OOBE completed...
            {
                m_completed = false; // <-- clear completion flag for this strategy, i.e. continue to display message.
            }
        }
    }

    if( !m_completed )
    {
        String assetName;
        if( parameters.hasValue( _assetName ) )
        {
            assetName = parameters[_assetName];
        }
        else
        {
            assetName = _message;
        }

        WindowManager::TerminalWindow terminalWindow;
        if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
        {
            m_terminal = terminalWindow.m_terminal;
            AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, assetName ) );
            if( assetLoader->open() )
            {
                m_terminal->clearScreen();
                m_windowManager.setTerminalWindowVisible( m_windowName, true );

                Assets::ANSIFile ansiFile( *assetLoader );
                m_terminal->consumeAsset( ansiFile,
                                        ansiFile.dataSize(),
                                        Terminal::scrollLock );

                m_timer->reset();
                m_lastSecond = 0;
                drawCountdown();
            }
            else
            {
                m_completed = true;
            }
            delete( assetLoader );
        }
        else
        {
            m_completed = true;
        }
    }
}

void Message::returnTo( const Value& parameters )
{
}

bool Message::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool Message::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        m_windowManager.setTerminalWindowVisible( m_windowName, false );
        return true;
    }

    return false;
}

void Message::run()
{
    if( m_completed || !m_terminal ) return;

    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }

        if( c == '\n' )
        {
            m_completed = true;
        }
    }

    long ms( m_timer->ms() );
    if( ms >= timeout )
    {
        m_completed = true;
    }
    else if( ( ms / 1000 ) > m_lastSecond )
    {
        drawCountdown();
        m_lastSecond = ms / 1000;
    }
}

void Message::drawCountdown()
{
    long timeRemain( ( timeout / 1000 ) - ( m_timer->ms() / 1000 ) );
    LiteStream stream;
    stream << ANSITerminal::colour( Terminal::colWhite )
           << "Ret to close or wait ";
    if( timeRemain < 10 )
    {
        stream << "\xFF"; // nbsp
    }
    stream << timeRemain
           << " second";
    if( timeRemain > 1 )
    {
        stream << "s";
    }
    else
    {
        stream << "\xFF"; // nbsp
    }
    stream << ANSITerminal::reset();
    m_terminal->printFormatted( stream.str(),
                                m_terminal->height() - 1,
                                0,
                                1,
                                m_terminal->width(),
                                Terminal::hCentre,
                                Terminal::noVCentre,
                                Terminal::colGrey,
                                Terminal::preserveBackground );
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
