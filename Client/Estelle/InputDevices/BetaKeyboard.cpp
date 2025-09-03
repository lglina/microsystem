#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/LiteStream.h"
#include "BetaKeyboard.h"

#include <xc.h>

namespace
{
    const int debounceTime( 20 );
    const int escTime( 2000 );
    const int repeatDelay( 750 );
    const int repeatPeriod( 100 );
} // Anonymous namespace

namespace Agape
{

namespace InputDevices
{

namespace
{
    char scanCodes[56] = {
        '\x1b', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '0', '-', '\x08', '\t', 'q', 'w',
        'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
        '=', '[', '\x00', 'a', 's', 'd', 'f', 'g',
        'h', 'j', 'k', 'l', ';', '\'', '\r', '\x00',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
        '.', '\x80', '/', ' ', '\x82', '\x81', '\x83'
    };

    char shiftedScanCodes[56] = {
        '\x1b', '!', '@', '#', '$', '%', '^', '&',
        '*', '(', ')', '_', '\x08', '\x84', 'Q', 'W',
        'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
        '+', ']', '\x00', 'A', 'S', 'D', 'F', 'G',
        'H', 'J', 'K', 'L', ':', '"', '\r', '\x00',
        'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
        '>', '\x85', '?', ' ', '\x87', '\x86', '\x88'
    };

    const int shiftRow( 4 );
    const int shiftCol( 7 );

    const int ctrlRow( 3 );
    const int ctrlCol( 2 );

    const int escRow( 0 );
    const int escCol( 0 );
} // Anonymous namespace

BetaKeyboard::BetaKeyboard( Timers::Factory& timerFactory ) :
  m_curRow( 0 ),
  m_curCol( 0 ),
  m_activeRow( -1 ),
  m_activeCol( -1 ),
  m_shiftTimer( timerFactory.makeTimer() ),
  m_ctrlTimer( timerFactory.makeTimer() ),
  m_escTimer( timerFactory.makeTimer() ),
  m_keyTimer( timerFactory.makeTimer() ),
  m_triggered( false ),
  m_lastRepeat( 0 ),
  m_shiftHeld( false ),
  m_ctrlHeld( false ),
  m_escPressed( false ),
  m_escHeld( false ),
  m_buffer( 8 )
{
}

BetaKeyboard::~BetaKeyboard()
{
    delete( m_shiftTimer );
    delete( m_ctrlTimer );
    delete( m_escTimer );
    delete( m_keyTimer );
}

bool BetaKeyboard::eof()
{
    return m_buffer.isEmpty();
}

char BetaKeyboard::peek()
{
    char c( '\0' );
    if( !m_buffer.isEmpty() )
    {
        c = m_buffer.front();
    }
    return c;
}

char BetaKeyboard::get()
{
    char c( '\0' );
    if( !m_buffer.isEmpty() )
    {
        c = m_buffer.pop();
    }
    return c;
}

void BetaKeyboard::run()
{
    // Assume pull-ups already configured for column pins.
    // Assume all row bits in LATA set low.
    TRISA = 0xFFFF & ( ~( 1 << ( 8 + m_curRow ) ) ); // Columns pull up, deselected rows high-Z.

    // Settle?
    if( ( m_curRow == shiftRow ) && ( m_curCol == shiftCol ) )
    {
        if( ( ( PORTA & 0x00FF ) & ( 1 << m_curCol ) ) == 0 )
        {
            if( m_shiftTimer->ms() >= debounceTime )
            {
                m_shiftHeld = true;
            }
        }
        else
        {
            m_shiftHeld = false;
            m_shiftTimer->reset();
        }
    }
    else if( ( m_curRow == ctrlRow ) && ( m_curCol == ctrlCol ) )
    {
        if( ( ( PORTA & 0x00FF ) & ( 1 << m_curCol ) ) == 0 )
        {
            if( m_ctrlTimer->ms() >= debounceTime )
            {
                m_ctrlHeld = true;
            }
        }
        else
        {
            m_ctrlHeld = false;
            m_ctrlTimer->reset();
        }
    }
    else if( ( m_curRow == escRow ) && ( m_curCol == escCol ) )
    {
        if( ( ( PORTA & 0x00FF ) & ( 1 << m_curCol ) ) == 0 )
        {
            // Escape being pressed
            if( !m_escPressed )
            {
                m_escTimer->reset();
            }
            m_escPressed = true;
            
            if( m_escTimer->ms() >= escTime )
            {
                // Esc has been held down at least 2s.
                m_escHeld = true;
            }
        }
        else if( m_escPressed )
        {
            // Escape was pressed and now released
            if( ( m_escTimer->ms() >= debounceTime ) && ( m_escTimer->ms() < escTime ) )
            {
                // Esc was pressed but not held - generate literal escape.
                m_buffer.push( '\x1b' );
            }
            m_escPressed = false;
            m_escHeld = false;
            m_escTimer->reset();
        }
    }
    else if( ( ( PORTA & 0x00FF ) & ( 1 << m_curCol ) ) == 0 )
    {
        // Key active in this row.
        if( m_activeCol != -1 )
        {
            // Key previously pressed.
            if( m_activeRow != m_curRow || m_activeCol != m_curCol )
            {
                // Different key now pressed.
                m_activeRow = m_curRow;
                m_activeCol = m_curCol;
                m_keyTimer->reset();
                m_triggered = false;
                m_lastRepeat = 0;
            }
            else
            {
                // Same key pressed.
                if( ( ( m_keyTimer->ms() >= debounceTime ) && !m_triggered ) ||
                    ( m_triggered &&
                      ( m_keyTimer->ms() >= repeatDelay ) &&
                      ( ( m_keyTimer->ms() - m_lastRepeat ) >= repeatPeriod ) ) )
                {
                    // Buffer key.
                    char c( 0 );
                    if( ( !m_shiftHeld ) && ( !m_ctrlHeld ) )
                    {
                        c = scanCodes[ ( m_activeRow * 8 ) + m_activeCol ];
                    }
                    else if( m_shiftHeld )
                    {
                        c = shiftedScanCodes[ ( m_activeRow * 8 ) + m_activeCol ];
                    }
                    else if( m_ctrlHeld )
                    {
                        c = scanCodes[ ( m_activeRow * 8 ) + m_activeCol ];
                        c -= 128;
                    }

#ifdef LOG_KEYB
                    {
                    LiteStream stream;
                    stream << "Key: " << (int)c;
                    LOG_DEBUG( stream.str() );
                    }
#endif
                    
                    if( !m_buffer.isFull() ) m_buffer.push( c );
                    if( c == '\r' )
                    {
                        if( !m_buffer.isFull() ) m_buffer.push( '\n' );
                    }

                    m_triggered = true;
                    m_lastRepeat = m_keyTimer->ms();
                }
            }
        }
        else
        {
            // New key now pressed.
            m_activeRow = m_curRow;
            m_activeCol = m_curCol;
            m_keyTimer->reset();
            m_triggered = false;
            m_lastRepeat = 0;
        }
    }
    else
    {
        // Key not active in this row.
        if( m_activeRow == m_curRow && m_activeCol == m_curCol )
        {
            // Key no longer pressed.
            m_activeRow = -1;
            m_activeCol = -1;
            m_keyTimer->reset();
            m_triggered = false;
            m_lastRepeat = 0;
        }
    }

    ++m_curCol;
    if( m_curCol == 8 )
    {
        m_curCol = 0;
        ++m_curRow;
        if( m_curRow == 7 )
        {
            m_curRow = 0;
        }
    }
}

bool BetaKeyboard::escHeld()
{
    if( m_escHeld )
    {
        m_escPressed = false;
        m_escHeld = false;
        m_escTimer->reset();
        return true;
    }
    
    return false;
}

} // namespace InputDevices

} // namespace Agape
