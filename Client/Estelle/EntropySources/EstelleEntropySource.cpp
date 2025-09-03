#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/RingBuffer.h"
#include "EstelleEntropySource.h"
#include "PICSerial.h"

#include <xc.h>

namespace Agape
{

namespace EntropySources
{

Estelle::Estelle( Timers::Factory& timerFactory,
                  PICSerial& picSerial ) :
  m_timer( timerFactory.makeTimer() ),
  m_picSerial( picSerial ),
  m_entropyPool( 64 ),
  m_shiftIn( 0 ),
  m_shiftCount( 0 ),
  m_count( 0 ) // REMOVE ME
{
    CCP2CON1bits.MOD = 0x01; // Single compare mode.
    CCP2CON2bits.OCAEN = 1; // Enable A and B outputs.
    CCP2CON2bits.OCBEN = 1;
    CCP2CON3bits.OUTM = 1; // Push-pull output mode.
    CCP2PR = 30; // Period. Adjusted to create ~12.5V output from CW under load (before R60), so Zener has some headroom.
    CCP2RA = 30; // 100% duty cycle (push-pull will alternate A and B outputs).

    CM2CONbits.CCH = 3;
    T2CONbits.TCS = 0;
}

Estelle::~Estelle()
{
    delete( m_timer );
}

int Estelle::generate( char* buffer, int len )
{
    int numToRead( m_entropyPool.size() < len ? m_entropyPool.size() : len );

    for( int i = 0; i < numToRead; ++i )
    {
        *buffer++ = m_entropyPool.pop();
    }

    return numToRead;
}

void Estelle::run()
{
    int bit1 = PORTCbits.RC0;
    m_timer->usleep( 10 );
    int bit2 = PORTCbits.RC0;
    m_timer->usleep( 10 );

    if( bit1 != bit2 )
    {
        m_shiftIn |= ( bit1 << m_shiftCount++ );
    }

    if( m_shiftCount == 8 )
    {
        if( m_entropyPool.isFull() ) m_entropyPool.pop();
        m_entropyPool.push( m_shiftIn );
        m_shiftIn = 0;
        m_shiftCount = 0;
    }
    
    // TESTING
    /*
    while( 1 )
    {
    int bit1 = PORTCbits.RC0;
    //m_timer->usleep( 10 );
    int bit2 = PORTCbits.RC0;
    //m_timer->usleep( 10 );

    if( bit1 != bit2 )
    {
        m_shiftIn |= ( bit1 << m_shiftCount++ );
    }

    if( m_shiftCount == 8 )
    {
        //if( m_entropyPool.isFull() ) m_entropyPool.pop();
        //m_entropyPool.push( m_shiftIn );
        m_picSerial.write( m_shiftIn );
        m_shiftIn = 0;
        m_shiftCount = 0;
        if( m_count < 100 ) m_count++;
        if( m_count == 100 )
        {
            LOG_DEBUG( "              " );
            ++m_count;
        }
    }
    if( m_count < 100 ) break;
    }
    */
}

void Estelle::setPowerState( enum PowerState powerState )
{
    if( powerState == PowerState::on )
    {
        CCP2CON1bits.ON = 1;
    }
    else if( powerState == PowerState::off )
    {
        CCP2CON1bits.ON = 0;
    }
}

} // namespace EntropySources

} // namespace Agape
