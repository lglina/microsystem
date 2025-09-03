#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "InputDevices/InputDevice.h"
#include "Platforms/Platform.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/LiteStream.h"
#include "Utils/StrToHex.h"
#include "Allocator.h"
#include "Collections.h"
#include "PlatformUI.h"
#include "String.h"
#include "StringConstants.h"
#include "TabBar.h"
#include "Terminal.h"
#include "WindowManager.h"

#include <math.h>

namespace
{
    const int updateInterval( 30000 ); // ms
    const double criticalBatteryPct( 20.0 );

    const int powerTabWidth( 6 );
    const int memoryTabWidth( 7 );

    const int batteryNormalAttr( 0x1F ); // White on blue
    const int batteryExtPwrAttr( 0x2F ); // White on green
    const int batteryErrorAttr( 0x70 ); // Black on grey
    const int batteryCriticalAttr( 0xCF ); // White on bright red
} // Anonymous namespace

namespace Agape
{

namespace UI
{

PlatformUI::PlatformUI( Agape::Platform& platform,
                        WindowManager& windowManager,
                        const String& modalWindowName,
                        TabBar& tabBar,
                        InputDevice& inputDevice,
                        Timers::Factory& timerFactory ) :
  m_platform( platform ),
  m_windowManager( windowManager ),
  m_tabBar( tabBar ),
  m_inputDevice( inputDevice ),
  m_modalTerminal( nullptr ),
  m_graphicsWarningShown( false ),
  m_peakHeapUsed( 0 )
{
    WindowManager::TerminalWindow terminalWindow;
    if( windowManager.getTerminalWindow( modalWindowName, terminalWindow ) )
    {
        m_modalTerminal = terminalWindow.m_terminal;
    }

    m_updateTimer = timerFactory.makeTimer();
    m_modalTimer = timerFactory.makeTimer();

    m_tabBar.create( _Power,
                     powerTabWidth,
                     UI::TabBar::right,
                     true, // true == visible
                     "\x03 ---%",
                     batteryErrorAttr );
    m_tabBar.create( _Memory,
                     memoryTabWidth,
                     UI::TabBar::right,
                     true, // true == visible
                     "---KiB",
                     0x1F );

    m_platform.registerListener( this );
}

PlatformUI::~PlatformUI()
{
    delete( m_updateTimer );
    delete( m_modalTimer );
}

void PlatformUI::receiveEvent( const Platform::Event& event )
{
    if( event.m_type == Platform::powerStateChanged )
    {
        struct Platform::PowerState powerState( m_platform.powerState() );
        updatePowerState( powerState );
    }
}

void PlatformUI::run()
{
    if( m_updateTimer->ms() >= updateInterval )
    {
        if( m_platform.error() )
        {
            Vector< Agape::Platform::ErrorType > errors;
            m_platform.currentErrors( errors );
            Vector< Agape::Platform::ErrorType >::const_iterator it( errors.begin() );
            for( ; it != errors.end(); ++it )
            {
                if( *it == Agape::Platform::displayError )
                {
                    if( !m_graphicsWarningShown )
                    {
                        //showGraphicsWarning();
                        m_graphicsWarningShown = true;
                    }
                }
            }
        }

        /*
        //m_notificationsTerminal->clearScreen();
        LiteStream stream;
        long heapUsed( m_platform.heapUsed() );
        if( heapUsed > m_peakHeapUsed ) m_peakHeapUsed = heapUsed;
        //stream << ( heapUsed / 1024 ) << "/" << ( m_peakHeapUsed / 1024 ) << " KiB heap bytes used/max";
        stream << "Heap KiB used/max/mxa "
               << ( heapUsed / 1024 ) << "/" << ( m_peakHeapUsed / 1024 ) << "/"
               << ( g_maxAlloc / 1024 ) << "@" << ulToHex( (unsigned long)g_maxAllocAddress );
        String message( stream.str() );
        //m_notificationsTerminal->consumeNext( 0, m_notificationsTerminal->width() - message.length() );
        //m_notificationsTerminal->consumeString( message, Terminal::scrollLock );
        */

        updateHeapState();

        m_updateTimer->reset();
    }
}

void PlatformUI::showAssetModal( const String& assetName )
{
    AssetLoaders::Baked assetLoader( World::Coordinates(), assetName );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        m_modalTerminal->transientBlank();
        m_modalTerminal->consumeNext( 0, 0 );
        m_modalTerminal->consumeAsset( ansiFile, ansiFile.dataSize(), Terminal::scrollLock, Terminal::transient );
    }

    char c( '\0' );
    while( c != '\n' )
    {
        m_modalTerminal->flush(); // Required for Qt, to enter a nested event loop to update screen and handle input events.
        m_inputDevice.run();
        while( !m_inputDevice.eof() && ( c != '\n' ) )
        {
            c = m_inputDevice.get();
        }

        m_modalTimer->usleep( 1000 ); // Don't burn up the CPU in Qt.
    }

    m_modalTerminal->repaint();
}

void PlatformUI::updatePowerState( struct Platform::PowerState& powerState )
{
    int attributes( batteryNormalAttr );
    String powerStr( "\x03 " );

    if( powerState.m_chargerState == Platform::battError )
    {
        attributes = batteryErrorAttr;
    }
    else if( ( powerState.m_chargerState == Platform::discharging ) &&
             ( powerState.m_batteryPct <= criticalBatteryPct ) )
    {
        attributes = batteryCriticalAttr;
    }
    else if( powerState.m_extPower )
    {
        attributes = batteryExtPwrAttr;
    }

    if( ( powerState.m_chargerState == Platform::charging ) ||
        ( powerState.m_chargerState == Platform::charged ) ||
        ( powerState.m_chargerState == Platform::discharging ) )
    {
        if( powerState.m_batteryPct < 100.0 ) powerStr += " ";
        if( powerState.m_batteryPct < 10.0 ) powerStr += " ";
        LiteStream stream;
        stream << ::lround( powerState.m_batteryPct );
        powerStr += stream.str();
    }
    else
    {
        powerStr += "---";
    }
    powerStr += "%";

    m_tabBar.update( _Power, powerStr, attributes );
}

void PlatformUI::updateHeapState()
{
    long heapUsed( m_platform.heapUsed() );
    LiteStream stream;
    if( heapUsed < 0x100000 )
    {
        stream << ( heapUsed / 1024 ) << "KiB";
    }
    else
    {
        stream << ( heapUsed / 0x100000 ) << "MiB";
    }

    m_tabBar.update( _Memory, stream.str() );
}

void PlatformUI::showGraphicsWarning()
{
    showAssetModal( "mcdp" );
}

} // namespace UI

} // namespace Agape
