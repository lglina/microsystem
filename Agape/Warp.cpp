#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/LiteStream.h"
#include "String.h"
#include "Warp.h"

namespace Agape
{

Timers::Factory* Warp::s_timerFactory( nullptr );

Warp::Warp( const String& taskName, bool doEngage ) :
  m_taskName( taskName ),
  m_timer( nullptr ),
  m_engaged( false )
{
    if( doEngage )
    {
        engage();
    }
}

Warp::~Warp()
{
    if( m_timer )
    {
        delete( m_timer );
    }
}

void Warp::engage()
{
    if( !m_engaged )
    {
        if( !m_timer && s_timerFactory )
        {
            m_timer = s_timerFactory->makeTimer();
        }

        if( m_timer )
        {
            LiteStream stream;
            stream << "\xF0\x9F\x9A\x80 \x1b[41m" << m_taskName << " started\x1b[49m";
            LOG_DEBUG( stream.str() );
            m_timer->reset();
            m_engaged = true;
        }
    }
}

void Warp::report()
{
    if( m_timer )
    {
        long elapsed( m_timer->us() );
        LiteStream stream;
        stream << "\xF0\x9F\x9A\x80 \x1b[42m" << m_taskName << " completed in " << elapsed << "us\x1b[49m";
        LOG_DEBUG( stream.str() );
        m_engaged = false;
    }
}

void Warp::setTimerFactory( Timers::Factory* timerFactory )
{
    s_timerFactory = timerFactory;
}

} // namespace Agape
