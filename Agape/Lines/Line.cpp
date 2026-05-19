#include "Line.h"
#include "LineDrivers/LineDriver.h"

namespace Agape
{

Line::Line( LineDriver& lineDriver ) :
  m_lineDriver( lineDriver ),
  m_lineStatusCounter( 0 )
{
}

Line::~Line()
{
    // NOP
}

void Line::open()
{
    m_lineDriver.open();
}

void Line::run()
{
    m_lineDriver.run();
}

int Line::read( char* data, int len )
{
    return m_lineDriver.read( data, len );
}

int Line::write( const char* data, int len )
{
    return m_lineDriver.write( data, len );
}

bool Line::error()
{
    return m_lineDriver.error();
}

void Line::setRequiresAuthentication( bool requiresAuthentication )
{
    m_requiresAuthentication = requiresAuthentication;
}

bool Line::requiresAuthentication() const
{
    return m_requiresAuthentication;
}

void Line::enableFlowControl( bool enable )
{
    m_lineDriver.enableFlowControl( enable );
}

int Line::controlLines()
{
    return m_lineDriver.controlLines();
}

void Line::setControlLines( int mask )
{
    m_lineDriver.setControlLines( mask );
}

void Line::clearControlLines( int mask )
{
    m_lineDriver.clearControlLines( mask );
}

void Line::enableLoopTest( bool enable )
{
    // NOP
}

} // namespace Agape
