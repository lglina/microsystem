#include "Collections.h"
#include "Lines/Line.h"
#include "Lines/DirectLine.h"
#include "LineDrivers/LineDriver.h"
#include "Utils/LiteStream.h"
#include "Utils/Tokeniser.h"
#include "String.h"
#include <stddef.h>

#include "Loggers/Logger.h"

namespace Agape
{

namespace Lines
{

Direct::Direct( LineDriver& lineDriver, bool dialSetsLinkAddress ) :
  Line( lineDriver ),
  m_dialSetsLinkAddress( dialSetsLinkAddress ),
  m_readyDelayTimer( 0 ),
  m_ringingDelayTimer( 0 )
{
}

void Direct::run()
{
    if( m_lineDriver.error() )
    {
        m_lineStatus.m_ready = false;
        m_lineStatus.m_ringing = false;
        m_lineStatus.m_carrier = false;;
        m_lineStatus.m_secure = false;
        m_readyDelayTimer = 0;
        m_ringingDelayTimer = 0;
    }
    
    if( m_readyDelayTimer > 0 )
    {
        ++m_readyDelayTimer;
        if( m_readyDelayTimer == 20 )
        {
            m_lineStatus.m_ready = true;
            m_readyDelayTimer = 0;
        }
    }

    if( m_lineStatus.m_ringing )
    {
        ++m_ringingDelayTimer;
        if( m_ringingDelayTimer == 20 )
        {
            m_lineStatus.m_ringing = false;
            m_lineStatus.m_carrier = true;
            m_lineStatus.m_secure = true;
            m_ringingDelayTimer = 0;
        }
    }

    m_lineDriver.run();
}

Vector< Line::ConfigOption > Direct::getConfigOptions()
{
    return Vector< ConfigOption >();
}

void Direct::setConfigOption( const String& name, const String& value )
{
}

void Direct::connect()
{
    m_readyDelayTimer = 1;
}

void Direct::registerNumber( const String& number )
{
}

void Direct::dial( const String& number )
{
    if( m_dialSetsLinkAddress )
    {
        m_lineDriver.setLinkAddress( number );
    }

    m_lineStatus.m_ringing = true;
}

void Direct::answer()
{
}

void Direct::hangup()
{
    m_lineStatus.m_carrier = false;
    m_lineStatus.m_ringing = false;
}

struct Line::LineStatus Direct::getLineStatus()
{
    return m_lineStatus;
}

} // namespace Lines

} // namespace Agape
