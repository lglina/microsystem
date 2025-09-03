#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/RingBuffer.h"
#include "SPIEntropySource.h"
#include "SPIRequester.h"
#include "SPIRequests.h"

namespace
{
    const int bufferSize = 128;
    const int requestSize = 8;
    const int pollTime = 500; // ms
    const int waitTime = 5; // ms
} // Anonymous namespace

namespace Agape
{

namespace EntropySources
{

SPI::SPI( SPIRequester& spiRequester,
          Timers::Factory& timerFactory ) :
  m_spiRequester( spiRequester ),
  m_timer( timerFactory.makeTimer() ),
  m_entropyPool( bufferSize ),
  m_requestSent( false )
{
}

SPI::~SPI()
{
    delete( m_timer );
}

int SPI::generate( char* buffer, int len )
{
    int numFilled( 0 );
    for( ; ( numFilled < len ) && !m_entropyPool.isEmpty(); ++numFilled )
    {
        buffer[numFilled] = m_entropyPool.pop();
    }

    return numFilled;
}

int SPI::poolSize()
{
    return bufferSize;
}

int SPI::poolRemain()
{
    return m_entropyPool.size();
}

void SPI::run()
{
    if( !m_requestSent &&
        ( m_timer->ms() >= pollTime ) &&
        !m_spiRequester.busy() )
    {
        char size = requestSize;
        m_spiRequester.sendRequest( SPIReadEntropy,
                                    &size,
                                    1 );
        m_requestSent = true;
        m_timer->reset();
    }
    else if( m_requestSent &&
             ( m_timer->ms() >= waitTime ) )
    {
        char response[maxSPIPayloadLength];
        int responseLength( m_spiRequester.readResponse( response ) );

        for( int i = 0; i < responseLength; ++i )
        {
            if( m_entropyPool.isFull() ) m_entropyPool.pop();
            m_entropyPool.push(response[i]);
        }

        m_requestSent = false;
        m_timer->reset();
    }
}

} // namespace EntropySources

} // namespace Agape
