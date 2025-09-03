#include "Direction.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"

namespace Agape
{

namespace World
{

Direction::Direction() :
  m_direction( none )
{
}

Direction::Direction( enum _Direction direction ) :
  m_direction( direction )
{
}

void Direction::toValue( Value& value ) const
{
    switch(m_direction)
    {
    case up:
        value = _up;
        break;
    case down:
        value = _down;
        break;
    case left:
        value = _left;
        break;
    case right:
        value = _right;
        break;
    default:
        value = _none;
        break;
    }
}

Direction Direction::fromValue( const Value& value )
{
    Direction direction;
    if( value == String( _up ) )
    {
        direction.m_direction = up;
    }
    if( value == String( _down ) )
    {
        direction.m_direction = down;
    }
    if( value == String( _left ) )
    {
        direction.m_direction = left;
    }
    if( value == String( _right ) )
    {
        direction.m_direction = right;
    }
    if( value == String( _none ) )
    {
        direction.m_direction = none;
    }

    return direction;
}

} // namespace World

} // namespace Agape
