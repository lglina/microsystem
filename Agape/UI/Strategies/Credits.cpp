#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "Assets/Asset.h"
#include "InputDevices/InputDevice.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "World/WorldCoordinates.h"
#include "Credits.h"
#include "String.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"

using namespace Agape;

namespace
{
    const int lineDelay( 150 ); // ms
    const int screenDelays[10] = {  1000, 15000, 15000, 15000, 20000,
                                   15000, 15000, 15000, 50000, 10000 };
    const String assetName( "credits" );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

Credits::Credits( InputDevice& inputDevice,
                  WindowManager& windowManager,
                  const String& windowName,
                  Timers::Factory& timerFactory ) :
  m_inputDevice( inputDevice ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_timer( timerFactory.makeTimer() ),
  m_completed( false ),
  m_terminal( nullptr ),
  m_currentAssetLoader( nullptr ),
  m_currentAsset( nullptr ),
  m_currentLine( 0 ),
  m_assetOffset( 0 ),
  m_screenNumber( 0 ),
  m_next( false )
{
}

Credits::~Credits()
{
    delete( m_timer );
    delete( m_currentAssetLoader );
    delete( m_currentAsset );
}

void Credits::enter( const Value& parameters )
{
    m_completed = false;
    m_timer->reset();

    m_currentAssetLoader = new AssetLoaders::Baked( World::Coordinates(), assetName );
    if( m_currentAssetLoader->open() )
    {
        m_currentAsset = new Assets::ANSIFile( *m_currentAssetLoader );
    }
    else
    {
        m_completed = true; // Uh oh!
    }
    m_currentLine = 0;
    m_assetOffset = 0;
    m_screenNumber = 0;

    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
        m_terminal->clearScreen();
    }
    else
    {
        m_completed = true; // Uh oh!
    }
}

void Credits::returnTo( const Value& parameters )
{
}

bool Credits::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool Credits::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        if( m_terminal ) m_terminal->clearScreen();
        if( m_currentAssetLoader )
        {
            delete( m_currentAsset );
            m_currentAsset = nullptr;

            m_currentAssetLoader->close();
            delete( m_currentAssetLoader );
            m_currentAssetLoader = nullptr;
        }
        return true;
    }

    return false;
}

void Credits::run()
{
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }
        else if( c == '\n' )
        {
            m_next = true;
        }
        else if( c == '\x1b' )
        {
            m_completed = true;
        }
    }

    if( ( ( m_currentLine % m_terminal->height() == 0 ) &&
          ( ( m_timer->ms() >= screenDelays[m_screenNumber] ) ||
            m_next ) ) ||
        ( ( m_currentLine == 0 ) || 
          ( ( m_currentLine % m_terminal->height() != 0 ) &&
            ( m_timer->ms() >= lineDelay ) ) ) )
    {
        if( m_terminal && m_currentAsset )
        {
            if( ( m_currentLine % m_terminal->height() == 0 ) &&
                ( m_currentLine != 0 ) )
            {
                m_currentLine = 0;
                ++m_screenNumber;

                if( m_screenNumber == 10 ) m_completed = true;
            }

            m_terminal->clearLines( m_currentLine, 1 );
            m_assetOffset += m_terminal->consumeAsset( *m_currentAsset,
                                                      m_assetOffset,
                                                      ( m_currentAsset->dataSize() - m_assetOffset ),
                                                      m_currentAsset->width(),
                                                      0,
                                                      ++m_currentLine,
                                                      Terminal::scrollLock );
        }
        m_timer->reset();
        m_next = false;
    }
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
