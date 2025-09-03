#include "Line.h"
#include "LineDrivers/LineDriver.h"

namespace Agape
{

Line::Line( LineDriver& lineDriver ) :
  m_lineDriver( lineDriver ),
  m_lineStatusCounter( 0 )
{
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

} // namespace Agape
