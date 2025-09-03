#include "InputDevices/InputDevice.h"
#include "Memories/Memory.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/LiteStream.h"
#include "MemoryStrategy.h"
#include "ReadableWritable.h"
#include "String.h"
#include "Terminal.h"
#include "WindowManager.h"

namespace Agape
{

namespace UI
{

namespace Strategies
{

Memory::Memory( WindowManager& windowManager,
                const String& windowName,
                InputDevice& inputDevice,
                Agape::Memory& memory,
                ReadableWritable& serialPort,
                Timers::Factory& timerFactory ) :
  m_inputDevice( inputDevice ),
  m_memory( memory ),
  m_serialPort( serialPort ),
  m_terminal( nullptr ),
  m_delayTimer( timerFactory.makeTimer() )
{
    WindowManager::TerminalWindow terminalWindow;
    if( windowManager.getTerminalWindow( windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }
}

Memory::~Memory()
{
    delete( m_delayTimer );
}

void Memory::enter( const Value& parameters )
{
    m_terminal->clearScreen();
    m_terminal->consumeString( "Flash Utility\r\n" );
    m_terminal->consumeString( "-------------\r\n" );
    m_terminal->consumeString( "Ret - Dump memory to debug serial\r\n" );
    m_terminal->consumeString( "X - Erase memory\r\n" );
}

void Memory::returnTo( const Value& parameters )
{
}

bool Memory::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool Memory::returning( String& nextStrategy, Value& parameters )
{
    return false;
}

void Memory::run()
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
            char buffer[256];
            int size( m_memory.size() );
            for( int offset = 0; offset < size; offset += 16 )
            {
                if( ( offset % 1024 ) == 0 )
                {
                    LiteStream stream;
                    stream << offset << " / " << size << "\r";
                    m_terminal->consumeString( stream.str(), Terminal::scrollLock );
                }
                m_memory.read( offset, buffer, 16 );
                m_serialPort.write( buffer, 16 );
                m_delayTimer->usleep(1280);
            }
            m_terminal->consumeString( " Done.\r\n" );
        }
        else if( c == 'x' )
        {
            m_terminal->consumeString( "Wait...\r\n" );
            m_memory.erase();
            m_terminal->consumeString( "Erased.\r\n" );
        }
    }
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
