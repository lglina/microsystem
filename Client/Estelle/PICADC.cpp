#include "Timers/Factories/TimerFactory.h"
#include "Collections.h"
#include "PICADC.h"
#include "Runnable.h"

#include <xc.h>

namespace
{
    const int acqTime( 50 ); // uS
    const int convTimeout( 50 ); // uS
} // Anonymous namespace

namespace Agape
{

PICADC::PICADC( Timers::Factory& timerFactory ) :
  m_timer( timerFactory.makeTimer() ),
  m_currentChannelIdx( -1 )
{
    // TAD = 2.25uS @ 8MHz PBCLK with ADCS = 8.
    // Tconv = 12 TAD = 27uS.
    // Tacq = 22uS, assuming max. 1M source impedance and 4.4pF Chold.
    AD1CON3bits.ADCS = 8; // No divider needed for TAD >= 200ns for 10-bit mode, per DS param AD50A.
    AD1CON1bits.ON = 1;

    // FIXME: We could set 12 bit resolution here, but need to be mindful of
    // the silicon errata applicable to 12-bit mode.
}

PICADC::~PICADC()
{
    delete( m_timer );
}

void PICADC::addChannel( int channelNum )
{
    struct Channel channel;
    channel.m_channelNum = channelNum;
    channel.m_value = 0;
    channel.m_haveSampled = false;
    m_channels.push_back( channel );
}

bool PICADC::getValue( int channelNum, int& value )
{
    // Just use a direct index rather than an iterator, for speed.
    for( int idx = 0; idx < m_channels.size(); ++idx )
    {
        const struct Channel& thisChannel( m_channels[idx] );
        if( ( thisChannel.m_channelNum == channelNum ) &&
            thisChannel.m_haveSampled )
        {
            value = thisChannel.m_value;
            return true;
        }
    }

    return false;
}

void PICADC::run()
{
    if( m_channels.empty() ) return;

    if( m_currentChannelIdx >= 0 )
    {
        if( m_timer->us() >= acqTime )
        {
            // Sampling done.
            struct Channel& thisChannel( m_channels[m_currentChannelIdx] );
            AD1CON1bits.DONE = 0;
            AD1CON1bits.SAMP = 0; // Start conversion
            m_timer->reset();
            while( ( AD1CON1bits.DONE == 0 ) &&
                   ( m_timer->us() < convTimeout ) ) {} // Wait for conversion to complete or timeout
            if( AD1CON1bits.DONE == 1 )
            {
                thisChannel.m_value = ADC1BUF0;
                thisChannel.m_haveSampled = true;
            }

            // Select next channel and start sampling.
            ++m_currentChannelIdx;
            if( m_currentChannelIdx >= m_channels.size() )
            {
                m_currentChannelIdx = 0;
            }
            AD1CHSbits.CH0SA = m_channels[m_currentChannelIdx].m_channelNum;
            AD1CON1bits.SAMP = 1;
            m_timer->reset();
        }
    }
    else
    {
        // Select first channel and start sampling.
        AD1CHSbits.CH0SA = m_channels[0].m_channelNum;
        AD1CON1bits.SAMP = 1;
        m_timer->reset();
        m_currentChannelIdx = 0;
    }
}

} // namespace Agape
