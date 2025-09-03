#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "Audio/MIDIPlayer.h"
#include "EntropySources/EntropySource.h"
#include "InputDevices/InputDevice.h"
#include "Memories/Memory.h"
#include "Platforms/Platform.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/StrToHex.h"
#include "Utils/LiteStream.h"
#include "World/WorldCoordinates.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "TestStrategy.h"
#include "WindowManager.h"

#include <memory.h>
#include <string.h>

namespace Agape
{

using namespace Audio;
using namespace InputDevices;
using namespace World;

namespace UI
{

namespace Strategies
{

Test::Test( WindowManager& windowManager,
            const String& windowName,
            InputDevice& inputDevice,
            Platform& platform,
            Agape::Memory& memory,
            MIDIPlayer& midiPlayer,
            EntropySource& entropySource,
            Timers::Factory& timerFactory ) :
  m_inputDevice( inputDevice ),
  m_platform( platform ),
  m_memory( memory ),
  m_midiPlayer( midiPlayer ),
  m_entropySource( entropySource ),
  m_terminal( nullptr ),
  m_timer( timerFactory.makeTimer() ),
  m_state( TestScreen )
{
    WindowManager::TerminalWindow terminalWindow;
    if( windowManager.getTerminalWindow( windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }
}

Test::~Test()
{
    delete( m_timer );
}

void Test::enter( const Value& parameters )
{
    setState();
}

void Test::returnTo( const Value& parameters )
{
}

bool Test::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool Test::returning( String& nextStrategy, Value& parameters )
{
    return false;
}

void Test::run()
{
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }

        if( c == '\n' )
        {
            switch( m_state )
            {
            case TestScreen:
                m_state = TestPowerSupply;
                break;
            case TestPowerSupply:
                m_state = TestKeyboard;
                break;
            case TestKeyboard:
                m_state = TestFlash;
                break;
            case TestFlash:
                m_state = TestModem;
                break;
            case TestModem:
                m_state = TestSound;
                break;
            case TestSound:
                m_state = TestRNG;
                break;
            case TestRNG:
            default:
                m_state = TestScreen;
                break;
            }
            setState();
        }

        if( m_state == TestScreen )
        {
            if( c == control( Key::up ) )
            {
                m_platform.brightnessUp();
            }
            else if( c == control( Key::down ) )
            {
                m_platform.brightnessDown();
            }
        }
        else if( m_state == TestPowerSupply )
        {
            if( c == Key::up )
            {
                m_platform.notify( Platform::newTelegram, Platform::client );
            }
            else if( c == Key::down )
            {
                m_platform.cancelNotify( Platform::newTelegram );
            }
        }
        else if( m_state == TestKeyboard )
        {
            if( c != '\n' )
            {
                m_terminal->consumeString( ucharToHex( *( (unsigned char*)( &c ) ) ) );
                m_terminal->consumeChar( ' ' );
                m_terminal->consumeChar( c, Terminal::scrollUnlock, Terminal::literal );
                m_terminal->consumeString( "\r\n" );
            }
        }
        else if( m_state == TestFlash )
        {
            if( c == 'x' )
            {
                m_terminal->consumeString( "Writing to flash\r\n" );
                m_memory.seek( 0 );
                m_memory.write( _Agape, ::strlen( _Agape ) );
                m_terminal->consumeString( "Reading from flash\r\n" );
                char test[::strlen( _Agape )];
                m_memory.seek( 0 );
                m_memory.read( test, ::strlen( _Agape ) );
                if( ::memcmp( _Agape, test, ::strlen( _Agape ) ) == 0 )
                {
                    m_terminal->consumeString( "PASSED\r\n" );
                }
                else
                {
                    m_terminal->consumeString( "FAILED\r\n" );
                }
                m_terminal->consumeString( "Erasing flash\r\n" );
                m_memory.erase();
                m_terminal->consumeString( "Erased.\r\n" );
            }
        }
        else if( m_state == TestSound )
        {
            if( c == 'p' )
            {
                m_midiPlayer.doPlay( Coordinates(), "promenade", true ); // true = loop.
            }
            else if( c == 's' )
            {
                m_midiPlayer.doStop( true ); // true = hard stop.
            }
        }
    }

    switch( m_state )
    {
    case TestPowerSupply:
        if( m_timer->ms() >= 500 )
        {
            char sensorData[4];
            m_platform.readSensors( sensorData, 4 ); // Queues read, returns current data. Ignore returned sensorData, as internalState() will print it for us.
            m_terminal->consumeString( m_platform.internalState() );
            m_timer->reset();
        }
        break;
    case TestRNG:
        if( m_timer->ms() >= 500 )
        {
            char c;
            if( m_entropySource.generate( &c, 1 ) == 1 )
            {
                m_terminal->consumeString( ucharToHex( *( (unsigned char*)( &c ) ) ) );
            }
            int currentRow( m_terminal->row() );
            int currentCol( m_terminal->col() );
            m_terminal->consumeNext( 0, 0, 0x0F );
            LiteStream stream;
            stream << "Pool bytes " << m_entropySource.poolRemain() << "/" << m_entropySource.poolSize();
            m_terminal->consumeString( stream.str() );
            m_terminal->consumeNext( currentRow, currentCol, 0x0F );
            
            m_timer->reset();
        }
        break;
    default:
        break;
    }
}

void Test::setState()
{
    switch( m_state )
    {
    case TestScreen:
        {
        m_terminal->clearScreen();
        AssetLoaders::Baked bakedAssetLoader( World::Coordinates(), "testpattern" );
        if( bakedAssetLoader.open() )
        {
            Assets::ANSIFile ansiFile( bakedAssetLoader );
            m_terminal->consumeAsset( ansiFile, ansiFile.dataSize(), Terminal::scrollLock );
        }
        }
        break;
    case TestPowerSupply:
        m_terminal->clearScreen();
        m_terminal->consumeString( "Platform controller test\r\nUp=alert, Down=cancel\r\n" );
        m_timer->reset();
        break;
    case TestKeyboard:
        m_terminal->clearScreen();
        m_terminal->consumeString( "Keyboard test\r\n" );
        break;
    case TestFlash:
        m_terminal->clearScreen();
        m_terminal->consumeString( "Flash test\r\nX to start\r\nWARNING: This will erase all flash memory\r\n" );
        break;
    case TestModem:
        m_terminal->clearScreen();
        m_terminal->consumeString( "Modem test\r\n" );
        // TODO
        break;
    case TestSound:
        m_terminal->clearScreen();
        m_terminal->consumeString( "Sound test\r\nP to play\r\nS to stop\r\n" );
        // TODO
        break;
    case TestRNG:
        m_terminal->clearScreen();
        m_terminal->consumeString( "RNG test\r\n" );
        m_timer->reset();
        break;
    default:
        break;
    }
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
