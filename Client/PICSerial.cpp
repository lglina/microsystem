#include "PICSerial.h"
#include "Utils/RingBuffer.h"

#include <math.h>
#include <xc.h>

#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"

#include "InterruptHandler.h"

#include "cpu.h"

namespace Agape
{

PICSerial::PICSerial( int port,
                      int baud,
                      int transmitBufferSize,
                      int receiveBufferSize,
                      bool flowControl,
                      bool debug ) :
  m_port( port ),
  m_transmitBuffer( transmitBufferSize ),
  m_receiveBuffer( receiveBufferSize ),
  m_flowControl( flowControl ),
  m_transmitHighWater( 0 ),
  m_transmitBufferFulls( 0 ),
  m_receiveHighWater( 0 ),
  m_receiveFIFOOverflows( 0 ),
  m_receiveBufferOverflows( 0 ),
  m_receiveErrors( 0 ),
  m_debug( debug ),
  m_needLog( false )
{
    // Prerequisite: ANSEL, TRIS, I/O remapping and
    // interrupt priority set by caller.

    int brg( ::round( ( (double)PBCLK_FREQ / ( 4 * baud ) ) - 1 ) );

    if( port == 1 )
    {
        U1MODEbits.BRGH = 1;
        U1BRG = brg;

        if( flowControl )
        {
            U1MODEbits.UEN = 0x02;
        }

        U1STAbits.UTXISEL = 0x02; // Interrupt when transmit buffer empty.
        //U1STAbits.URXISEL = 0x01; // Interrupt when receive buffer 1/2 full.
        U1STAbits.UTXEN = 1;
        U1STAbits.URXEN = 1;
        U1MODEbits.ON = 1;

        setRXIE();
    }
    else if( port == 2 )
    {
        U2MODEbits.BRGH = 1;
        U2BRG = brg;

        if( flowControl )
        {
            U2MODEbits.UEN = 0x02;
        }

        U2STAbits.UTXISEL = 0x02; // Interrupt when transmit buffer empty.
        //U2STAbits.URXISEL = 0x01; // Interrupt when receive buffer 1/2 full.
        U2STAbits.UTXEN = 1;
        U2STAbits.URXEN = 1;
        U2MODEbits.ON = 1;

        setRXIE();
    }
    else if( port == 3 )
    {
        U3MODEbits.BRGH = 1;
        U3BRG = brg;

        if( flowControl )
        {
            U3MODEbits.UEN = 0x02;
        }

        U3STAbits.UTXISEL = 0x02; // Interrupt when transmit buffer empty.
        //U3STAbits.URXISEL = 0x01; // Interrupt when receive buffer 1/2 full.
        U3STAbits.UTXEN = 1;
        U3STAbits.URXEN = 1;
        U3MODEbits.ON = 1;

        setRXIE();
    }
#if defined(__PIC32MZ__) || defined(__PIC32MX__)
    else if( port == 4 )
    {
        U4MODEbits.BRGH = 1;
        U4BRG = brg;

        if( flowControl )
        {
            U4MODEbits.UEN = 0x02;
        }

        U4STAbits.UTXISEL = 0x02; // Interrupt when transmit buffer empty.
        //U4STAbits.URXISEL = 0x01; // Interrupt when receive buffer 1/2 full.
        U4STAbits.UTXEN = 1;
        U4STAbits.URXEN = 1;
        U4MODEbits.ON = 1;

        setRXIE();
    }
    else if( port == 5 )
    {
        U5MODEbits.BRGH = 1;
        U5BRG = brg;

        if( flowControl )
        {
            U5MODEbits.UEN = 0x02;
        }

        U5STAbits.UTXISEL = 0x02; // Interrupt when transmit buffer empty.
        //U5STAbits.URXISEL = 0x01; // Interrupt when receive buffer 1/2 full.
        U5STAbits.UTXEN = 1;
        U5STAbits.URXEN = 1;
        U5MODEbits.ON = 1;

        setRXIE();
    }
#endif
}

int PICSerial::read( char* data, int len )
{
    char* dataPtr( data );
    int lenRead( 0 );

    if( m_debug && m_needLog )
    {
        doLog();
        m_needLog = false;
    }

    while( !m_receiveBuffer.isEmpty() && lenRead < len )
    {
        *dataPtr++ = m_receiveBuffer.pop();
        ++lenRead;
    }

    setRXIE();

    return lenRead;
}

int PICSerial::write( const char* data, int len )
{
    const char* dataPtr( data );
    int lenWritten( 0 );

    while( !m_transmitBuffer.isFull() && lenWritten < len )
    {
        m_transmitBuffer.push( *dataPtr++ );
        int transmitSize( m_transmitBuffer.size() );
        if( transmitSize > m_transmitHighWater ) { m_transmitHighWater = transmitSize; }
        ++lenWritten;
    }
    
    if( m_transmitBuffer.isFull() )
    {
        m_needLog = true;
        ++m_transmitBufferFulls;
    }

    setTXIE();

    if( m_debug && m_needLog )
    {
        doLog();
        m_needLog = false;
    }

    return lenWritten;
}

bool PICSerial::error()
{
    return false;
}

void PICSerial::flushInput()
{
    m_receiveBuffer.clear();
}

int PICSerial::write( const char datum )
{
    return write( &datum, 1 );
}

bool PICSerial::eof() const
{
    return m_receiveBuffer.isEmpty();
}

void PICSerial::handleInterrupt( enum InterruptDispatcher::InterruptVector vector )
{
#if defined(__PIC32MZ__)
    if( ( m_port == 1 && IFS3bits.U1TXIF ) ||
        ( m_port == 2 && IFS4bits.U2TXIF ) ||
        ( m_port == 3 && IFS4bits.U3TXIF ) ||
        ( m_port == 4 && IFS5bits.U4TXIF ) ||
        ( m_port == 5 && IFS5bits.U5TXIF ) )
#elif defined(__PIC32MX__)
    if( ( m_port == 1 && IFS1bits.U1TXIF ) ||
        ( m_port == 2 && IFS1bits.U2TXIF ) ||
        ( m_port == 3 && IFS2bits.U3TXIF ) ||
        ( m_port == 4 && IFS2bits.U4TXIF ) ||
        ( m_port == 5 && IFS2bits.U5TXIF ) )
#elif defined(__PIC32MM__)
    if( ( vector == InterruptDispatcher::UART1Tx || vector == InterruptDispatcher::UART2Tx ) &&
        ( ( m_port == 1 && IFS1bits.U1TXIF ) || ( m_port == 2 && IFS1bits.U2TXIF ) ) )
#endif
	{
		transmitISR();
	}

#if defined(__PIC32MZ__)
    if( ( m_port == 1 && IFS3bits.U1RXIF ) ||
        ( m_port == 2 && IFS4bits.U2RXIF ) ||
        ( m_port == 3 && IFS4bits.U3RXIF ) ||
        ( m_port == 4 && IFS5bits.U4RXIF ) ||
        ( m_port == 5 && IFS5bits.U5RXIF ) )
#elif defined(__PIC32MX__)
	if( ( m_port == 1 && IFS1bits.U1RXIF ) ||
        ( m_port == 2 && IFS1bits.U2RXIF ) ||
        ( m_port == 3 && IFS1bits.U3RXIF ) ||
        ( m_port == 4 && IFS2bits.U4RXIF ) ||
        ( m_port == 5 && IFS2bits.U5RXIF ) )
#elif defined(__PIC32MM__)
    if( ( vector == InterruptDispatcher::UART1Rx || vector == InterruptDispatcher::UART2Rx ) &&
        ( ( m_port == 1 && IFS1bits.U1RXIF ) || ( m_port == 2 && IFS1bits.U2RXIF ) ) )
#endif
	{
		receiveISR();
	}
}

void PICSerial::transmitISR()
{
    while( !m_transmitBuffer.isEmpty() && 
           ( ( m_port == 1 && !U1STAbits.UTXBF ) ||
#if defined(__PIC32MZ__) || defined(__PIC32MX__)
             ( m_port == 2 && !U2STAbits.UTXBF ) ||
             ( m_port == 3 && !U3STAbits.UTXBF ) ||
             ( m_port == 4 && !U4STAbits.UTXBF ) ||
             ( m_port == 5 && !U5STAbits.UTXBF ) ) )
#else
             ( m_port == 2 && !U2STAbits.UTXBF ) ) )
#endif
	{
        if( m_port == 1 )
        {
            U1TXREG = m_transmitBuffer.pop();
        }
        else if( m_port == 2 )
        {
            U2TXREG = m_transmitBuffer.pop();
        }
        else if( m_port == 3 )
        {
            U3TXREG = m_transmitBuffer.pop();
        }
#if defined(__PIC32MZ__) || defined(__PIC32MX__)
        else if( m_port == 4 )
        {
            U4TXREG = m_transmitBuffer.pop();
        }
        else if( m_port == 5 )
        {
            U5TXREG = m_transmitBuffer.pop();
        }
#endif
    }

    clearTXIF();

    if( m_transmitBuffer.isEmpty() )
    {
        clearTXIE();
    }
}

void PICSerial::receiveISR()
{
    while( !m_receiveBuffer.isFull() && 
           ( ( m_port == 1 && U1STAbits.URXDA ) ||
#if defined(__PIC32MZ__) || defined(__PIC32MX__)
             ( m_port == 2 && U2STAbits.URXDA ) ||
             ( m_port == 3 && U3STAbits.URXDA ) ||
             ( m_port == 4 && U4STAbits.URXDA ) ||
             ( m_port == 5 && U5STAbits.URXDA ) ) )
#else
             ( m_port == 2 && U2STAbits.URXDA ) ) )
#endif
	{
		if( m_port == 1 && U1STAbits.PERR )
		{
            U1STAbits.PERR = 0;
            ++m_receiveErrors;
            m_needLog = true;
		}
        else if( m_port == 2 && U2STAbits.PERR )
		{
            U2STAbits.PERR = 0;
            ++m_receiveErrors;
            m_needLog = true;
		}
        else if( m_port == 3 && U3STAbits.PERR )
		{
            U3STAbits.PERR = 0;
            ++m_receiveErrors;
            m_needLog = true;
		}
#if defined(__PIC32MZ__) || defined(__PIC32MX__)
        else if( m_port == 4 && U4STAbits.PERR )
		{
            U4STAbits.PERR = 0;
            ++m_receiveErrors;
            m_needLog = true;
		}
        else if( m_port == 5 && U5STAbits.PERR )
		{
            U5STAbits.PERR = 0;
            ++m_receiveErrors;
            m_needLog = true;
		}
#endif

        if( m_port == 1 && U1STAbits.FERR )
		{
            U1STAbits.FERR = 0;
            ++m_receiveErrors;
            m_needLog = true;
		}
        else if( m_port == 2 && U2STAbits.FERR )
		{
            U2STAbits.FERR = 0;
            ++m_receiveErrors;
            m_needLog = true;
		}
        else if( m_port == 3 && U3STAbits.FERR )
		{
            U3STAbits.FERR = 0;
            ++m_receiveErrors;
            m_needLog = true;
		}
#if defined(__PIC32MZ__) || defined(__PIC32MX__)
        else if( m_port == 4 && U4STAbits.FERR )
		{
            U4STAbits.FERR = 0;
            ++m_receiveErrors;
            m_needLog = true;
		}
        else if( m_port == 5 && U5STAbits.FERR )
		{
            U5STAbits.FERR = 0;
            ++m_receiveErrors;
            m_needLog = true;
		}
#endif

        if( m_port == 1 )
        {
            m_receiveBuffer.push( U1RXREG );
        }
        else if( m_port == 2 )
        {
            m_receiveBuffer.push( U2RXREG );
        }
        else if( m_port == 3 )
        {
            m_receiveBuffer.push( U3RXREG );
        }
#if defined(__PIC32MZ__) || defined(__PIC32MX__)
        else if( m_port == 4 )
        {
            m_receiveBuffer.push( U4RXREG );
        }
        else if( m_port == 5 )
        {
            m_receiveBuffer.push( U5RXREG );
        }
#endif

        int receiveSize( m_receiveBuffer.size() );
        if( receiveSize > m_receiveHighWater ) { m_receiveHighWater = receiveSize; }
    }
    
    if( m_receiveBuffer.isFull() &&
        ( ( m_port == 1 && U1STAbits.URXDA ) ||
#if defined(__PIC32MZ__) || defined(__PIC32MX__)
          ( m_port == 2 && U2STAbits.URXDA ) ||
          ( m_port == 3 && U3STAbits.URXDA ) ||
          ( m_port == 4 && U4STAbits.URXDA ) ||
          ( m_port == 5 && U5STAbits.URXDA ) ) )
#else
          ( m_port == 2 && U2STAbits.URXDA ) ) )
#endif
    {
        // Characters still in FIFO but our buffer is full.

        // Despite clearing RXIF it will assert again as we haven't emptied
        // the FIFO by reading RXREG. Clear RXIE to prevent constant interrupts
        // and we can re-enable it when a character is read from the receive
        // buffer with read(). If we're using flow control, the EUSART will
        // assert RTS to get the other end to stop transmitting, which it
        // hopefully will do before the FIFO fills up. If we're not using flow
        // control, we'll lose characters if the FIFO overflows.

        if( !m_flowControl )
        {
            ++m_receiveBufferOverflows;
            m_needLog = true;
            //__builtin_software_breakpoint();
        }

        clearRXIE();
    }

    // TODO: Handle?
	if( m_port == 1 && U1STAbits.OERR )
    {
        U1STAbits.OERR = 0;
        ++m_receiveFIFOOverflows;
        m_needLog = true;
        //__builtin_software_breakpoint();
    }
    else if( m_port == 2 && U2STAbits.OERR )
	{
		U2STAbits.OERR = 0;
        ++m_receiveFIFOOverflows;
        m_needLog = true;
        //__builtin_software_breakpoint();
	}
    else if( m_port == 3 && U3STAbits.OERR )
	{
		U3STAbits.OERR = 0;
        ++m_receiveFIFOOverflows;
        m_needLog = true;
        //__builtin_software_breakpoint();
	}
#if defined(__PIC32MZ__) || defined(__PIC32MX__)
    else if( m_port == 4 && U4STAbits.OERR )
	{
		U4STAbits.OERR = 0;
        ++m_receiveFIFOOverflows;
        m_needLog = true;
        //__builtin_software_breakpoint();
	}
    else if( m_port == 5 && U5STAbits.OERR )
	{
		U5STAbits.OERR = 0;
        ++m_receiveFIFOOverflows;
        m_needLog = true;
        //__builtin_software_breakpoint();
	}
#endif

    clearRXIF();
}

/// MZ
// RXIE 3 4 4 5 5
// TXIE 3 4 4 5 5

/// MX
// RXIE 1 1 1 2 2
// TXIE 1 1 2 2 2

/// MM
// RXIE 1 1 1
// TXIE 1 1 1

inline void PICSerial::setRXIE()
{
    if( m_port == 1 )
    {
#if defined(__PIC32MZ__)
        IEC3SET = _IEC3_U1RXIE_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IEC1SET = _IEC1_U1RXIE_MASK;
#endif
    }
    else if( m_port == 2 )
    {
#if defined(__PIC32MZ__)
        IEC4SET = _IEC4_U2RXIE_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IEC1SET = _IEC1_U2RXIE_MASK;
#endif
    }
    else if( m_port == 3 )
    {
#if defined(__PIC32MZ__)
        IEC4SET = _IEC4_U3RXIE_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IEC1SET = _IEC1_U3RXIE_MASK;
#endif
    }
    else if( m_port == 4 )
    {
#if defined(__PIC32MZ__)
        IEC5SET = _IEC5_U4RXIE_MASK;
#elif defined(__PIC32MX__)
        IEC2SET = _IEC2_U4RXIE_MASK;
#endif
    }
    else if( m_port == 5 )
    {
#if defined(__PIC32MZ__)
        IEC5SET = _IEC5_U5RXIE_MASK;
#elif defined(__PIC32MX__)
        IEC2SET = _IEC2_U5RXIE_MASK;
#endif
    }
}

inline void PICSerial::setTXIE()
{
    if( m_port == 1 )
    {
#if defined(__PIC32MZ__)
        IEC3SET = _IEC3_U1TXIE_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IEC1SET = _IEC1_U1TXIE_MASK;
#endif
    }
    else if( m_port == 2 )
    {
#if defined(__PIC32MZ__)
        IEC4SET = _IEC4_U2TXIE_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IEC1SET = _IEC1_U2TXIE_MASK;
#endif
    }
    else if( m_port == 3 )
    {
#if defined(__PIC32MZ__)
        IEC4SET = _IEC4_U3TXIE_MASK;
#elif defined(__PIC32MX__)
        IEC2SET = _IEC2_U3TXIE_MASK;
#elif defined(__PIC32MM__)
        IEC1SET = _IEC1_U3TXIE_MASK;
#endif
    }
    else if( m_port == 4 )
    {
#if defined(__PIC32MZ__)
        IEC5SET = _IEC5_U4TXIE_MASK;
#elif defined(__PIC32MX__)
        IEC2SET = _IEC2_U4TXIE_MASK;
#endif
    }
    else if( m_port == 5 )
    {
#if defined(__PIC32MZ__)
        IEC5SET = _IEC5_U5TXIE_MASK;
#elif defined(__PIC32MX__)
        IEC2SET = _IEC2_U5TXIE_MASK;
#endif
    }
}

inline void PICSerial::clearRXIE()
{
    if( m_port == 1 )
    {
#if defined(__PIC32MZ__)
        IEC3CLR = _IEC3_U1RXIE_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IEC1CLR = _IEC1_U1RXIE_MASK;
#endif
    }
    else if( m_port == 2 )
    {
#if defined(__PIC32MZ__)
        IEC4CLR = _IEC4_U2RXIE_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IEC1CLR = _IEC1_U2RXIE_MASK;
#endif
    }
    else if( m_port == 3 )
    {
#if defined(__PIC32MZ__)
        IEC4CLR = _IEC4_U3RXIE_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IEC1CLR = _IEC1_U3RXIE_MASK;
#endif
    }
    else if( m_port == 4 )
    {
#if defined(__PIC32MZ__)
        IEC5CLR = _IEC5_U4RXIE_MASK;
#elif defined(__PIC32MX__)
        IEC2CLR = _IEC2_U4RXIE_MASK;
#endif
    }
    else if( m_port == 5 )
    {
#if defined(__PIC32MZ__)
        IEC5CLR = _IEC5_U5RXIE_MASK;
#elif defined(__PIC32MX__)
        IEC2CLR = _IEC2_U5RXIE_MASK;
#endif
    }
}

inline void PICSerial::clearTXIE()
{
    if( m_port == 1 )
    {
#if defined(__PIC32MZ__)
        IEC3CLR = _IEC3_U1TXIE_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IEC1CLR = _IEC1_U1TXIE_MASK;
#endif
    }
    else if( m_port == 2 )
    {
#if defined(__PIC32MZ__)
        IEC4CLR = _IEC4_U2TXIE_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IEC1CLR = _IEC1_U2TXIE_MASK;
#endif
    }
    else if( m_port == 3 )
    {
#if defined(__PIC32MZ__)
        IEC4CLR = _IEC4_U3TXIE_MASK;
#elif defined(__PIC32MX__)
        IEC2CLR = _IEC2_U3TXIE_MASK;
#elif defined(__PIC32MM__)
        IEC1CLR = _IEC1_U3TXIE_MASK;
#endif
    }
    else if( m_port == 4 )
    {
#if defined(__PIC32MZ__)
        IEC5CLR = _IEC5_U4TXIE_MASK;
#elif defined(__PIC32MX__)
        IEC2CLR = _IEC2_U4TXIE_MASK;
#endif
    }
    else if( m_port == 5 )
    {
#if defined(__PIC32MZ__)
        IEC5CLR = _IEC5_U5TXIE_MASK;
#elif defined(__PIC32MX__)
        IEC2CLR = _IEC2_U5TXIE_MASK;
#endif
    }
}

/// MZ
// RXIF 3 4 4 5 5
// TXIF 3 4 4 5 5

/// MX
// RXIF 1 1 1 2 2
// TXIF 1 1 2 2 2

/// MM
// RXIF 1 1 1
// TXIF 1 1 1

inline void PICSerial::clearRXIF()
{
    if( m_port == 1 )
    {
#if defined(__PIC32MZ__)
        IFS3CLR = _IFS3_U1RXIF_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IFS1CLR = _IFS1_U1RXIF_MASK;
#endif
    }
    else if( m_port == 2 )
    {
#if defined(__PIC32MZ__)
        IFS4CLR = _IFS4_U2RXIF_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IFS1CLR = _IFS1_U2RXIF_MASK;
#endif
    }
    else if( m_port == 3 )
    {
#if defined(__PIC32MZ__)
        IFS4CLR = _IFS4_U3RXIF_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IFS1CLR = _IFS1_U3RXIF_MASK;
#endif
    }
    else if( m_port == 4 )
    {
#if defined(__PIC32MZ__)
        IFS5CLR = _IFS5_U4RXIF_MASK;
#elif defined(__PIC32MX__)
        IFS2CLR = _IFS2_U4RXIF_MASK;
#endif
    }
    else if( m_port == 5 )
    {
#if defined(__PIC32MZ__)
        IFS5CLR = _IFS5_U5RXIF_MASK;
#elif defined(__PIC32MX__)
        IFS2CLR = _IFS2_U5RXIF_MASK;
#endif
    }
}

inline void PICSerial::clearTXIF()
{
    if( m_port == 1 )
    {
#if defined(__PIC32MZ__)
        IFS3CLR = _IFS3_U1TXIF_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IFS1CLR = _IFS1_U1TXIF_MASK;
#endif
    }
    else if( m_port == 2 )
    {
#if defined(__PIC32MZ__)
        IFS4CLR = _IFS4_U2TXIF_MASK;
#elif defined(__PIC32MM__) || defined(__PIC32MX__)
        IFS1CLR = _IFS1_U2TXIF_MASK;
#endif
    }
    else if( m_port == 3 )
    {
#if defined(__PIC32MZ__)
        IFS4CLR = _IFS4_U3TXIF_MASK;
#elif defined(__PIC32MX__)
        IFS2CLR = _IFS2_U3TXIF_MASK;
#elif defined(__PIC32MM__)
        IFS1CLR = _IFS1_U3TXIF_MASK;
#endif
    }
    else if( m_port == 4 )
    {
#if defined(__PIC32MZ__)
        IFS5CLR = _IFS5_U4TXIF_MASK;
#elif defined(__PIC32MX__)
        IFS2CLR = _IFS2_U4TXIF_MASK;
#endif
    }
    else if( m_port == 5 )
    {
#if defined(__PIC32MZ__)
        IFS5CLR = _IFS5_U5TXIF_MASK;
#elif defined(__PIC32MX__)
        IFS2CLR = _IFS2_U5TXIF_MASK;
#endif
    }
}

void PICSerial::doLog()
{
    LiteStream stream;
    stream << "Ser" << m_port << ":"
           << " TX HW: " << m_transmitHighWater
           << " TX BF: " << m_transmitBufferFulls
           << " RX HW: " << m_receiveHighWater
           << " RX FOVR: " << m_receiveFIFOOverflows
           << " RX BOVR: " << m_receiveBufferOverflows
           << " RX ERR: " << m_receiveErrors;
    LOG_DEBUG( stream.str() );
}

} // namespace Agape
