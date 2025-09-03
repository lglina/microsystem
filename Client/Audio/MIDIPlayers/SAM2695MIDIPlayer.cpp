#include "Assets/Asset.h"
#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "Audio/FrobMIDIPlayer.h"
#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Utils/RingBuffer.h"
#include "World/WorldCoordinates.h"
#include "SAM2695MIDIPlayer.h"
#include "InterruptHandler.h"
#include "PICSerial.h"
#include "String.h"
#include "cpu.h"

#include <xc.h>

namespace Agape
{

namespace Audio
{

namespace MIDIPlayers
{

SAM2695::SAM2695( AssetLoaders::Factory& assetLoaderFactory, PICSerial& midiOut ) :
  FrobMIDIPlayer( assetLoaderFactory ),
  m_midiOut( midiOut ),
  m_frobClock( 0 ),
  m_nextFrobLength( 0 ),
  m_nextFrobTime( 0 ),
  m_havePrefix( false ),
  m_lateFrobs( 0 )
{
    PR1 = PBCLK_FREQ / 3906.25; // Divide by frobs per second.
    Agape::InterruptDispatcher::instance()->registerHandler( Agape::InterruptDispatcher::timer1, this );
}

void SAM2695::programChange( int channel, int instrument )
{
    char midiEvent[2];

    midiEvent[0] = 0xC0 + channel;
    midiEvent[1] = instrument;

    m_midiOut.write( midiEvent, 2 );
}

void SAM2695::playNote( int channel, int pitch, int velocity )
{
    if( m_state == playing )
    {
        doStop( true ); // true = hard.
    }

    char midiEvent[3];

    midiEvent[0] = 0x90 + channel;
    midiEvent[1] = pitch;
    midiEvent[2] = velocity;
    m_midiOut.write( midiEvent, 3 );
}

void SAM2695::handleInterrupt( enum InterruptDispatcher::InterruptVector vector )
{
    IFS0CLR = _IFS0_T1IF_MASK;

    if( !m_havePrefix )
    {
        getPrefix();
    }

    while( m_havePrefix && ( m_nextFrobTime <= m_frobClock ) )
    {
        if( m_frobBuffer.size() >= m_nextFrobLength ) // We have a complete frob.
        {
            if( m_nextFrobTime < m_frobClock )
            {
                ++m_lateFrobs;
            }

            for( int offset = 0; offset < m_nextFrobLength; ++offset )
            {
                char b( m_frobBuffer.pop() );
                // NB: UART ISR must have higher priority than us so it can
                // drain the serial buffer while we wait.
                while( m_midiOut.write( b ) != 1 ) {}
            }
        
            getPrefix();
        }
        else
        {
            // We don't have a complete frob. Wait until next time and
            // handle late.
            break;
        }
    }

    ++m_frobClock;
}

void SAM2695::playQueued()
{
    if( T1CONbits.ON == 0 )
    {
        TMR1 = 0;
        IFS0CLR = _IFS0_T1IF_MASK;
        IEC0SET = _IEC0_T1IE_MASK;
        T1CONbits.ON = 1;
    }
}

void SAM2695::stopPlayingQueued()
{
    T1CONbits.ON = 0;
    IEC0CLR = _IEC0_T1IE_MASK;
    IFS0CLR = _IFS0_T1IF_MASK;
    TMR1 = 0;

    m_frobClock = 0;
    m_nextFrobLength = 0;
    m_nextFrobTime = 0;
    m_havePrefix = false;
    m_lateFrobs = 0;

    m_midiOut.write( '\xFF' ); // MIDI Reset.
}

void SAM2695::getPrefix()
{
    if( m_frobBuffer.size() > 4 ) // We have a prefix (time and size).
    {
        char frob[4];
        frob[0] = m_frobBuffer.pop();
        frob[1] = m_frobBuffer.pop();
        frob[2] = m_frobBuffer.pop();
        frob[3] = m_frobBuffer.pop();

        m_nextFrobLength = *( (unsigned char*)(&frob[0]) );

        m_nextFrobTime =  ( *( (unsigned char*)(&frob[1]) ) ) << 16;
        m_nextFrobTime += ( *( (unsigned char*)(&frob[2]) ) ) << 8;
        m_nextFrobTime +=   *( (unsigned char*)(&frob[3]) );

        m_havePrefix = true;
    }
    else
    {
        m_havePrefix = false;
    }
}

} // namespace MIDIPlayers

} // namespace Audio

} // namespace Agape
