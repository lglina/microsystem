#include "Clocks/Clock.h"
#include "Snowflake.h"
#include "StrToHex.h"

namespace
{
    Agape::Snowflake* g_snowflakeInstance( nullptr );
} // Anonymous namespace

namespace Agape
{

Snowflake::Snowflake( Clock& clock, int machineID ) :
  m_clock( clock ),
  m_machineID( machineID ),
  m_sequence( 0 )
{
    g_snowflakeInstance = this;
}

Snowflake::~Snowflake()
{
    g_snowflakeInstance = nullptr;
}

String Snowflake::generate()
{
    if( g_snowflakeInstance != nullptr )
    {
        return g_snowflakeInstance->_generate();
    }
    else
    {
        return String();
    }
}

String Snowflake::_generate()
{
    unsigned long long snowflake( m_clock.epochMS() << 22 );
    snowflake += ( ( m_machineID & 0x03FF ) << 12 );
    snowflake += ( m_sequence & 0x0FFF );

    ++m_sequence;
    if( m_sequence > 0x0FFF )
    {
        m_sequence = 0;
    }

    return ullToHex( snowflake );
}

} // namespace Agape
