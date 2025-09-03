#include "NullLineDriver.h"

namespace Agape
{

namespace LineDrivers
{

Null::Null() :
  m_linkReady( false )
{
}

int Null::open()
{
    m_linkReady = true;
    return 0;
}

int Null::read( char* data, int len )
{
    return 0;
}

int Null::write( const char* data, int len )
{
    return len;
}

void Null::setLinkAddress()
{
}

bool Null::linkReady()
{
    return m_linkReady;
}

} // namespace LineDrivers

} // namespace Agape
