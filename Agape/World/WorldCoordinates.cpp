#include "String.h"
#include "StringConstants.h"
#include "Value.h"
#include "WorldCoordinates.h"

#ifndef NO_LINDA2
#include "TupleRoutingCriteria.h"
#endif

namespace Agape
{

namespace World
{

Coordinates::Coordinates() :
  m_x( 0 ),
  m_y( 0 )
{
}

Coordinates::Coordinates( const String& worldID ) :
  m_worldID( worldID ),
  m_x( 0 ),
  m_y( 0 )
{
}

Coordinates::Coordinates( const String& worldID, int x, int y ) :
  m_worldID( worldID ),
  m_x( x ),
  m_y( y )
{
}

bool Coordinates::operator==( const Coordinates& other ) const
{
    return( ( m_worldID == other.m_worldID ) &&
            ( m_x == other.m_x ) &&
            ( m_y == other.m_y ) );
}

bool Coordinates::operator!=( const Coordinates& other ) const
{
  return( !( operator==( other ) ) );
}

void Coordinates::toValue( Value& value, bool allScenes ) const
{
    value[_worldID] = m_worldID;
    if( !allScenes )
    {
      value[_x] = m_x;
      value[_y] = m_y;
    }
}

Coordinates Coordinates::fromValue( const Value& value )
{
    return Coordinates( value[_worldID], value[_x], value[_y] );
}

#ifndef NO_LINDA2
void Coordinates::toRoutingCriteria( Linda2::TupleRoutingCriteria& tupleRoutingCriteria,
                                     const char* valueName,
                                     bool allScenes ) const
{
    tupleRoutingCriteria.m_values[valueName][_worldID] = m_worldID;
    if( !allScenes )
    {
      tupleRoutingCriteria.m_values[valueName][_x] = m_x;
      tupleRoutingCriteria.m_values[valueName][_y] = m_y;
    }
}
#endif

} // namespace World

} // namespace Agape
