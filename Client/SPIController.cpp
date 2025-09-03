#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/LiteStream.h"
#include "Utils/StrToHex.h"
#include "InterruptHandler.h"
#include "SPIController.h"
#include "cpu.h"

#include <xc.h>

namespace
{
    const int timeout( 10 ); // ms
} // Anonymous namespace

namespace Agape
{

SPIController::SPIController( int peripheral,
                              int clockSpeed,
                              bool master,
                              Timers::Factory& timerFactory,
                              int transmitBufferSize,
                              int receiveBufferSize ) :
  m_peripheral( peripheral ),
  m_master( master ),
  m_timer( timerFactory.makeTimer() ),
  m_transmitBuffer( transmitBufferSize ),
  m_receiveBuffer( receiveBufferSize ),
  m_dummyReadCount( 0 )
{
    if( peripheral == 1 )
    {
        setOn( false );
        if( master )
        {
            SPI1CONbits.MSTEN = 1;
        }
        else
        {
            SPI1CONbits.SSEN = 1;
        }
        SPI1CONbits.CKP = 1; // Clock active-low.
        SPI1CONbits.CKE = 0; // Zeroes are the defaults, but set them to be explicit.
        SPI1CONbits.SMP = 0;
        SPI1CONbits.ENHBUF = 1; // Use 128b FIFOs
        SPI1CONbits.STXISEL = 0x03; // TXIF when TX buffer not full. Most conservative, but more interrupts.
        SPI1CONbits.SRXISEL = 0x01; // RXIF when RX buffer not empty. Most conservative, but more interrupts.
        SPI1BRG = ( PBCLK_FREQ / clockSpeed / 2 ) - 1;
        if( master ) setOn( true );
        setRXIE();
    }
    else if( peripheral == 2 )
    {
        setOn( false );
        if( master )
        {
            SPI2CONbits.MSTEN = 1;
        }
        else
        {
            SPI2CONbits.SSEN = 1;
        }
        SPI2CONbits.CKP = 1; // Clock active-low.
        SPI2CONbits.CKE = 0; // Zeroes are the defaults, but set them to be explicit.
        SPI2CONbits.SMP = 0;
        SPI2CONbits.ENHBUF = 1; // Use 128b FIFOs
        SPI2CONbits.STXISEL = 0x03; // TXIF when TX buffer not full. Most conservative, but more interrupts.
        SPI2CONbits.SRXISEL = 0x01; // RXIF when RX buffer not empty. Most conservative, but more interrupts.
        SPI2BRG = ( PBCLK_FREQ / clockSpeed / 2 ) - 1;
        if( master ) setOn( true );
        setRXIE();
    }
}

SPIController::~SPIController()
{
    delete( m_timer );
}

int SPIController::read( char* data, int len )
{
    /*
    LiteStream stream;
    stream << "SPIController: Read " << len << " bytes";
    LOG_DEBUG( stream.str() );
    */

    char* dataPtr( data );
    int lenRead( 0 );

    if( m_master )
    {
        // Make dummy writes to read data from slave.
        // Master reads are blocking due to the need to make dummy writes
        // and so the caller doesn't prematurely drop /CS.
        for( int i = 0; i < len; ++i )
        {
            if( m_peripheral == 1 )
            {
                while( SPI1STATbits.SPITBF ) {}
                SPI1BUF = 0x00;
            }
            else if( m_peripheral == 2 )
            {
                while( SPI2STATbits.SPITBF ) {}
                SPI2BUF = 0x00;
            }
        }

        // Wait for receive interrupt to receive all incoming data.
        m_timer->reset();
        while( ( m_timer->ms() < timeout ) && ( m_receiveBuffer.size() < len ) ) {}
    }

    // RX interrupt reads into ring buffer and we read that out here.
    while( !m_receiveBuffer.isEmpty() && lenRead < len )
    {
        *dataPtr++ = m_receiveBuffer.pop();
        ++lenRead;
    }

    setRXIE(); // In case we were disabled due to RX ring buffer full.

#ifdef LOG_SPI
    if( lenRead > 0 )
    {
        LiteStream stream;
        stream << "SPIController: Read " << lenRead << " bytes of " << len << " expected";
        LOG_DEBUG( stream.str() );
        hexDump( data, lenRead );
    }
#endif

    return lenRead;
}

int SPIController::write( const char* data, int len )
{
    /*
    LiteStream stream;
    stream << "SPIController: Write " << len << " bytes";
    LOG_DEBUG( stream.str() );
    */

    const char* dataPtr( data );
    int lenWritten( 0 );

    while( !m_transmitBuffer.isFull() && lenWritten < len )
    {
        m_transmitBuffer.push( *dataPtr++ );
        ++lenWritten;
    }

    setTXIE();

    if( m_master )
    {
        // Master writes are blocking so the caller doesn't prematurely
        // drop /CS.
        m_timer->reset();
        while( ( m_timer->ms() < timeout ) && // Not timed out, and
               ( !m_transmitBuffer.isEmpty() || // TX buffer not empty OR TX FIFO not empty OR shift register not empty
                 ( ( m_peripheral == 1 ) && ( !SPI1STATbits.SPITBE || !SPI1STATbits.SRMT ) ) ||
                 ( ( m_peripheral == 2 ) && ( !SPI2STATbits.SPITBE || !SPI2STATbits.SRMT ) )
               )
             )
        {
        }
    }

#ifdef LOG_SPI
    if( lenWritten > 0 )
    {
        LiteStream stream;
        stream << "SPIController: Wrote " << lenWritten << " bytes of " << len << " total";
        LOG_DEBUG( stream.str() );
        hexDump( data, lenWritten );
    }
#endif

    return lenWritten;
}

bool SPIController::error()
{
    return false;
}

int SPIController::write( const char data )
{
    write( &data, 1 );
    return 1;
}

bool SPIController::eof()
{
    return m_receiveBuffer.isEmpty();
}

void SPIController::setPowerState( enum PowerState powerState )
{
    if( powerState == PowerState::on )
    {
        LOG_DEBUG( "SPIController: Power ON" );
        flushInput();
        clearOverflow();
        m_transmitBuffer.clear();
        m_receiveBuffer.clear();
        m_dummyReadCount = 0;
        setOn( true );
        clearTXIF();
        clearRXIF();
        setRXIE();
    }
    else if( powerState == PowerState::off )
    {
        LOG_DEBUG( "SPIController: Power OFF" );
        clearTXIE();
        clearRXIE();
        setOn( false );
    }
}

void SPIController::handleInterrupt( enum InterruptDispatcher::InterruptVector vector )
{
#if defined(__PIC32MX__) || defined(__PIC32MM__)
    if( ( m_peripheral == 1 && IFS1bits.SPI1TXIF ) || ( m_peripheral == 2 && IFS1bits.SPI2TXIF ) )
	{
		transmitISR();
	}

	if( ( m_peripheral == 1 && IFS1bits.SPI1RXIF ) || ( m_peripheral == 2 && IFS1bits.SPI2RXIF ) )
	{
		receiveISR();
	}
#elif defined(__PIC32MZ__)
    if( ( m_peripheral == 1 && IFS3bits.SPI1TXIF ) || ( m_peripheral == 2 && IFS4bits.SPI2TXIF ) )
	{
		transmitISR();
	}

	if( ( m_peripheral == 1 && IFS3bits.SPI1RXIF ) || ( m_peripheral == 2 && IFS4bits.SPI2RXIF ) )
	{
		receiveISR();
	}
#endif
}

void SPIController::transmitISR()
{
    while( !m_transmitBuffer.isEmpty() && 
           ( ( m_peripheral == 1 && !SPI1STATbits.SPITBF ) || ( m_peripheral == 2 && !SPI2STATbits.SPITBF ) ) &&
           ( !m_master || ( m_dummyReadCount < 5 ) ) )
	{
        // N.B. At five pending dummy reads, get out of this loop and
        // go to the Receive ISR to start emptying out the receive FIFO,
        // else it will overflow. Don't do this for the slave, however, or we'll
        // be blocking on the master - assume that we'll only ever transmit
        // fewer bytes than the FIFO sizes (16B) and so we'll just fill up the
        // FIFO here and wait for the master to clock it out (and get into the
        // Receive ISR then to handle the dummy reads).
        if( m_peripheral == 1 )
        {
            SPI1BUF = m_transmitBuffer.pop();
            ++m_dummyReadCount;
        }
        else if( m_peripheral == 2 )
        {
            SPI2BUF = m_transmitBuffer.pop();
            ++m_dummyReadCount;
        }
    }

    clearTXIF();

    if( m_transmitBuffer.isEmpty() )
    {
        clearTXIE();
    }
}

void SPIController::receiveISR()
{
    while( !m_receiveBuffer.isFull() && 
           ( ( m_peripheral == 1 && !SPI1STATbits.SPIRBE ) || ( m_peripheral == 2 && !SPI2STATbits.SPIRBE ) ) )
	{
        if( ( m_peripheral == 1 ) && SPI1STATbits.SPIROV )
        {
            clearOverflow();
            LOG_DEBUG( "SPIController: SPI1: Receive FIFO overflow" );
        }
        else if( ( m_peripheral == 2 ) && SPI2STATbits.SPIROV )
        {
            clearOverflow();
            LOG_DEBUG( "SPIController: SPI2: Receive FIFO overflow" );
        }

        if( m_peripheral == 1 )
        {
            if( m_dummyReadCount == 0 )
            {
                m_receiveBuffer.push( SPI1BUF );
            }
            else
            {
                // Read in response to previous write. Throw away.
                int dummy = SPI1BUF;
                --m_dummyReadCount;
            }
        }
        else if( m_peripheral == 2 )
        {
            if( m_dummyReadCount == 0 )
            {
                m_receiveBuffer.push( SPI2BUF );
            }
            else
            {
                // Read in response to previous write. Throw away.
                int dummy = SPI2BUF;
                --m_dummyReadCount;
            }
        }
    }
    
    if( m_receiveBuffer.isFull() &&
        ( ( m_peripheral == 1 && !SPI1STATbits.SPIRBE ) || ( m_peripheral == 2 && !SPI2STATbits.SPIRBE ) ) )
    {
        // Characters still in FIFO but our buffer is full.

        // Despite clearing RXIF it will assert again as we haven't emptied
        // the receive buffer by reading SPIXBUF. Clear RXIE to prevent constant
        // interrupts and we can re-enable it when a character is read from the
        // receive buffer with read().

        if( m_peripheral == 1 )
        {
            LOG_DEBUG( "SPIController: SPI1: Receive buffer overflow" );
        }
        else if( m_peripheral == 2 )
        {
            LOG_DEBUG( "SPIController: SPI2: Receive buffer overflow" );
        }
        clearRXIE();
    }

    clearRXIF();
}

inline void SPIController::setRXIE()
{
#if defined(__PIC32MX__) || defined(__PIC32MM__)
    if( m_peripheral == 1 )
    {
        IEC1SET = _IEC1_SPI1RXIE_MASK;
    }
    else if( m_peripheral == 2 )
    {
        IEC1SET = _IEC1_SPI2RXIE_MASK;
    }
#elif defined(__PIC32MZ__)
    if( m_peripheral == 1 )
    {
        IEC3SET = _IEC3_SPI1RXIE_MASK;
    }
    else if( m_peripheral == 2 )
    {
        IEC4SET = _IEC4_SPI2RXIE_MASK;
    }
#endif
}

inline void SPIController::setTXIE()
{
#if defined(__PIC32MX__) || defined(__PIC32MM__)
    if( m_peripheral == 1 )
    {
        IEC1SET = _IEC1_SPI1TXIE_MASK;
    }
    else if( m_peripheral == 2 )
    {
        IEC1SET = _IEC1_SPI2TXIE_MASK;
    }
#elif defined(__PIC32MZ__)
    if( m_peripheral == 1 )
    {
        IEC3SET = _IEC3_SPI1TXIE_MASK;
    }
    else if( m_peripheral == 2 )
    {
        IEC4SET = _IEC4_SPI2TXIE_MASK;
    }
#endif
}

inline void SPIController::clearRXIE()
{
#if defined(__PIC32MX__) || defined(__PIC32MM__)
    if( m_peripheral == 1 )
    {
        IEC1CLR = _IEC1_SPI1RXIE_MASK;
    }
    else if( m_peripheral == 2 )
    {
        IEC1CLR = _IEC1_SPI2RXIE_MASK;
    }
#elif defined(__PIC32MZ__)
    if( m_peripheral == 1 )
    {
        IEC3CLR = _IEC3_SPI1RXIE_MASK;
    }
    else if( m_peripheral == 2 )
    {
        IEC4CLR = _IEC4_SPI2RXIE_MASK;
    }
#endif
}

inline void SPIController::clearTXIE()
{
#if defined(__PIC32MX__) || defined(__PIC32MM__)
    if( m_peripheral == 1 )
    {
        IEC1CLR = _IEC1_SPI1TXIE_MASK;
    }
    else if( m_peripheral == 2 )
    {
        IEC1CLR = _IEC1_SPI2TXIE_MASK;
    }
#elif defined(__PIC32MZ__)
    if( m_peripheral == 1 )
    {
        IEC3CLR = _IEC3_SPI1TXIE_MASK;
    }
    else if( m_peripheral == 2 )
    {
        IEC4CLR = _IEC4_SPI2TXIE_MASK;
    }
#endif
}

inline void SPIController::clearRXIF()
{
#if defined(__PIC32MX__) || defined(__PIC32MM__)
    if( m_peripheral == 1 )
    {
        IFS1CLR = _IFS1_SPI1RXIF_MASK;
    }
    else if( m_peripheral == 2 )
    {
        IFS1CLR = _IFS1_SPI2RXIF_MASK;
    }
#elif defined(__PIC32MZ__)
    if( m_peripheral == 1 )
    {
        IFS3CLR = _IFS3_SPI1RXIF_MASK;
    }
    else if( m_peripheral == 2 )
    {
        IFS4CLR = _IFS4_SPI2RXIF_MASK;
    }
#endif
}

inline void SPIController::clearTXIF()
{
#if defined(__PIC32MX__) || defined(__PIC32MM__)
    if( m_peripheral == 1 )
    {
        IFS1CLR = _IFS1_SPI1TXIF_MASK;
    }
    else if( m_peripheral == 2 )
    {
        IFS1CLR = _IFS1_SPI2TXIF_MASK;
    }
#elif defined(__PIC32MZ__)
    if( m_peripheral == 1 )
    {
        IFS3CLR = _IFS3_SPI1TXIF_MASK;
    }
    else if( m_peripheral == 2 )
    {
        IFS4CLR = _IFS4_SPI2TXIF_MASK;
    }
#endif
}

inline void SPIController::setOn( bool on )
{
    if( m_peripheral == 1 )
    {
        SPI1CONbits.ON = on;
    }
    else if( m_peripheral == 2 )
    {
        SPI2CONbits.ON = on;
    }
}

inline void SPIController::flushInput()
{
    if( m_peripheral == 1 )
    {
        int dummy = SPI1BUF;
    }
    else if( m_peripheral == 2 )
    {
        int dummy = SPI2BUF;
    }
}

inline void SPIController::clearOverflow()
{
    if( m_peripheral == 1 )
    {
        SPI1STATCLR = _SPI1STAT_SPIROV_MASK;
    }
    else if( m_peripheral == 2 )
    {
        SPI2STATCLR = _SPI2STAT_SPIROV_MASK;
    }
}

} // namespace Agape