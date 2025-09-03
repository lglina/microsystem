#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/LiteStream.h"
#include "Utils/RingBuffer.h"
#include "SPIKeyboard.h"
#include "SPIRequester.h"
#include "SPIRequests.h"

namespace
{
    const int bufferSize = 32;
    const int pollTime = 50; // ms
    const int waitTime = 5; // ms
} // Anonymous namespace

namespace Agape
{

namespace InputDevices
{

SPIKeyboard::SPIKeyboard( SPIRequester& spiRequester,
                          Timers::Factory& timerFactory ) :
  m_spiRequester( spiRequester ),
  m_timer( timerFactory.makeTimer() ),
  m_buffer( bufferSize ),
  m_requestSent( false )
{
}

SPIKeyboard::~SPIKeyboard()
{
    delete( m_timer );
}

bool SPIKeyboard::eof()
{
    return m_buffer.isEmpty();
}

char SPIKeyboard::peek()
{
    char c( '\0' );
    if( !m_buffer.isEmpty() )
    {
        c = m_buffer.front();
    }

    return c;
}

char SPIKeyboard::get()
{
    char c( '\0' );
    if( !m_buffer.isEmpty() )
    {
        c = m_buffer.pop();
    }

    return c;
}

void SPIKeyboard::run()
{
    if( !m_requestSent &&
        ( m_timer->ms() >= pollTime ) &&
        !m_spiRequester.busy() )
    {
#ifdef LOG_SPI
        LOG_DEBUG( "==Reading keyboard==" );
#endif
        m_spiRequester.sendRequest( SPIReadInput,
                                    nullptr,
                                    0 );
        m_requestSent = true;
        m_timer->reset();
    }
    else if( m_requestSent &&
             ( m_timer->ms() >= waitTime ) )
    {
        char response[maxSPIPayloadLength];
        int responseLength( m_spiRequester.readResponse( response ) );

        for( int i = 0; ( !m_buffer.isFull() ) && ( i < responseLength ); ++i )
        {
#ifdef LOG_KEYB
            {
            LiteStream stream;
            stream << "Key: " << (int)response[i];
            LOG_DEBUG( stream.str() );
            }
#endif
            m_buffer.push( response[i] );
        }

        if( m_buffer.isFull() )
        {
            LOG_DEBUG( "SPIKeyboard: Buffer overflow" );
        }

#ifdef LOG_SPI
        LOG_DEBUG( "==Reading keyboard done==" );
#endif
        m_requestSent = false;
        m_timer->reset();
    }
}

} // namespace InputDevices

} // namespace Agape
