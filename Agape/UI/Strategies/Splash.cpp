#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "InputDevices/InputDevice.h"
#include "Loggers/Logger.h"
#include "Platforms/Platform.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "World/WorldCoordinates.h"
#include "Splash.h"
#include "String.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"

#include "Warp.h"

namespace
{
    const int splashDelay( 1000 ); // ms.
    const int splashDelayLong( 5000 ); // ms.

    const int numAssets( 6 );
    const char* asset1( "splash" );
    const char* asset2( "aimm" );
    const char* asset3( "computer-reset" );
    const char* asset4( "playtronics" );
    const char* asset5( "templeos" );
    const char* asset6( "cardiff" );
    const char* assetNames[6] = { asset1, asset2, asset3, asset4, asset5, asset6 };
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

Splash::Splash( WindowManager& windowManager,
                InputDevice& inputDevice,
                Timers::Factory& timerFactory,
                Platform& platform ) :
  m_windowManager( windowManager ),
  m_inputDevice( inputDevice ),
  m_timer( timerFactory.makeTimer() ),
  m_platform( platform ),
  m_assetNumber( 0 ),
  m_splashDelay( splashDelay )
{
}

Splash::~Splash()
{
    delete( m_timer );
}

void Splash::enter( const Value& parameters )
{
    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( "Map", terminalWindow ) )
    {
        if( parameters.hasValue( _next ) )
        {
            ++m_assetNumber;
            if( m_assetNumber == numAssets ) m_assetNumber = 0;
            terminalWindow.m_terminal->clearScreen();
            m_splashDelay = splashDelayLong;
        }

        String assetName = assetNames[m_assetNumber];

        AssetLoaders::Baked assetLoader( World::Coordinates(), assetName );
        assetLoader.open();
        Assets::ANSIFile ansiFile( assetLoader );
        Warp w1( "Draw splash screen" );
        terminalWindow.m_terminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
        w1.report();
    }

    m_platform.performSelfTest();
    m_platform.doBootTasks();

    m_nextStrategy.clear();

    if( m_windowManager.getTerminalWindow( "Map", terminalWindow ) )
    {
        terminalWindow.m_terminal->consumeNext( 0, 0, 0x07 );
        terminalWindow.m_terminal->consumeChar( '*' );
    }

    m_timer->reset();
}

void Splash::returnTo( const Value& parameters )
{
}

bool Splash::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool Splash::returning( String& nextStrategy, Value& parameters )
{
    if( m_timer->ms() >= m_splashDelay )
    {
        checkForKey();

        if( m_nextStrategy.empty() )
        {
            nextStrategy = _menu;
        }
        else
        {
            nextStrategy = m_nextStrategy;
        }
        return true;
    }
    return false;
}

void Splash::run()
{
    checkForKey();
}

void Splash::checkForKey()
{
    m_inputDevice.run();
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }

        if( c == 'm' )
        {
            m_nextStrategy = "memory";
        }
        else if( c == 't' )
        {
            m_nextStrategy = "test";
        }
    }
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
