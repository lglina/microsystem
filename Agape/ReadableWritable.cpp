#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "ReadableWritable.h"
#include "String.h"

namespace
{
    const int defaultReadTimeout( 10000 ); // ms
} // Anonymous namespace

namespace Agape
{

Timers::Factory* ReadableWritable::s_timerFactory( nullptr );

ReadableWritable::ReadableWritable() :
  m_timer( nullptr )
{
}

ReadableWritable::~ReadableWritable()
{
    delete( m_timer );
}

int ReadableWritable::read( char* data, int len, bool blocking, int timeout )
{
    int currentTimeout = ( ( timeout > 0 ) ? timeout : defaultReadTimeout );

    if( blocking )
    {
        if( !m_timer && s_timerFactory ) m_timer = s_timerFactory->makeTimer();
        if( m_timer ) m_timer->reset();

        int numRead( 0 );
        bool error( false );
        while( !error && ( numRead < len ) && ( !m_timer || ( m_timer->ms() < currentTimeout ) ) )
        {
            int numRemain( len - numRead );
            int thisRead( read( data + numRead, numRemain, false ) );
            if( thisRead >= 0 )
            {
                numRead += thisRead;
            }
            else
            {
                error = true;
            }
        }

        return numRead;
    }
    else
    {
        return read( data, len );
    }
}

void ReadableWritable::readLine( String& s, int timeout )
{
    int currentTimeout = ( ( timeout > 0 ) ? timeout : defaultReadTimeout );

    if( !m_timer && s_timerFactory ) m_timer = s_timerFactory->makeTimer();
    if( m_timer ) m_timer->reset();

    // Read first char
    s.clear();
    char c;
    while( ( read( &c, 1 ) != 1 ) && ( !m_timer || ( m_timer->ms() < currentTimeout ) ) )
    {
    }

    // While current char is not newline add to string.
    while( ( c != '\n' ) && ( !m_timer || ( m_timer->ms() < currentTimeout ) ) )
    {
        // Eat all carriage returns.
        if( c != '\r' )
        {
            s += c;
        }

        // Read next char.
        while( ( read( &c, 1 ) != 1 ) && ( !m_timer || ( m_timer->ms() < currentTimeout ) ) )
        {
        }
    }
}

void ReadableWritable::setTimerFactory( Timers::Factory* timerFactory )
{
    s_timerFactory = timerFactory;
}

} // namespace Agape
