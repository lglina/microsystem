#include "Utils/LiteStream.h"
#include "Cartesian.h"
#include "String.h"

namespace Agape
{

Point::Point() :
  m_x( 0 ),
  m_y( 0 )
{
}

Point::Point( int x, int y ) :
  m_x( x ),
  m_y( y )
{
}

Rectangle::Rectangle()
{
}

Rectangle::Rectangle( const Point& upperLeft, const Point& bottomRight ) :
  m_upperLeft( upperLeft ),
  m_bottomRight( bottomRight )
{
}

Rectangle::Rectangle( const Point& origin, int height, int width ) :
  m_upperLeft( origin.m_x, origin.m_y ),
  m_bottomRight( ( origin.m_x + width - 1 ), ( origin.m_y + height - 1 ) )
{
}

Rectangle::Rectangle( int x, int y, int height, int width ) :
  m_upperLeft( x, y ),
  m_bottomRight( ( x + width - 1 ), ( y + height - 1 ) )
{
}

bool Rectangle::intersects( const Rectangle& other ) const
{
    return( !( ( m_upperLeft.m_x > other.m_bottomRight.m_x ) ||
               ( m_bottomRight.m_x < other.m_upperLeft.m_x ) ||
               ( m_upperLeft.m_y > other.m_bottomRight.m_y ) ||
               ( m_bottomRight.m_y < other.m_upperLeft.m_y ) ) );
}

Rectangle Rectangle::findIntersection( const Rectangle& other ) const
{
    int startX = ( other.m_upperLeft.m_x < m_upperLeft.m_x ) ? m_upperLeft.m_x : other.m_upperLeft.m_x;
    int endX = ( other.m_bottomRight.m_x > m_bottomRight.m_x ) ? m_bottomRight.m_x : other.m_bottomRight.m_x;
    int startY = ( other.m_upperLeft.m_y < m_upperLeft.m_y ) ? m_upperLeft.m_y : other.m_upperLeft.m_y;
    int endY = ( other.m_bottomRight.m_y > m_bottomRight.m_y ) ? m_bottomRight.m_y : other.m_bottomRight.m_y;

    return Rectangle( Point( startX, startY ), Point( endX, endY ) );
}

bool Rectangle::pointIntersects( const Point& point, int rectExpansion ) const
{
    return( ( point.m_x >= ( m_upperLeft.m_x - rectExpansion ) ) &&
            ( point.m_x <= ( m_bottomRight.m_x + rectExpansion ) ) &&
            ( point.m_y >= ( m_upperLeft.m_y - rectExpansion ) ) &&
            ( point.m_y <= ( m_bottomRight.m_y + rectExpansion ) ) );
}

Rectangle Rectangle::findUnionBoundingBox( const Rectangle& other ) const
{
    int startX = ( m_upperLeft.m_x < other.m_upperLeft.m_x ) ? m_upperLeft.m_x : other.m_upperLeft.m_x;
    int endX = ( m_bottomRight.m_x > other.m_bottomRight.m_x ) ? m_bottomRight.m_x : other.m_bottomRight.m_x;
    int startY = ( m_upperLeft.m_y < other.m_upperLeft.m_y ) ? m_upperLeft.m_y : other.m_upperLeft.m_y;
    int endY = ( m_bottomRight.m_y > other.m_bottomRight.m_y ) ? m_bottomRight.m_y : other.m_bottomRight.m_y;

    return Rectangle( Point( startX, startY ), Point( endX, endY ) );
}

enum Rectangle::Edge Rectangle::edgeTouches( const Rectangle& other ) const
{
    int startX = m_upperLeft.m_x;
    int endX = m_bottomRight.m_x;
    int startY = m_upperLeft.m_y;
    int endY = m_bottomRight.m_y;
    int otherStartX = other.m_upperLeft.m_x;
    int otherEndX = other.m_bottomRight.m_x;
    int otherStartY = other.m_upperLeft.m_y;
    int otherEndY = other.m_bottomRight.m_y;

    if( ( otherEndY == ( startY - 1 ) ) &&
        !( ( otherEndX < startX ) || ( otherStartX > endX ) ) )
    {
        return edgeTop;
    }
    else if( ( otherStartY == ( endY + 1 ) ) &&
             !( ( otherEndX < startX ) || ( otherStartX > endX ) ) )
    {
        return edgeBottom;
    }
    else if( ( otherEndX == ( startX - 1 ) ) &&
             !( ( otherEndY < startY ) || ( otherStartY > endY ) ) )
    {
        return edgeLeft;
    }
    else if( ( otherStartX == ( endX + 1 ) ) &&
             !( ( otherEndY < startY ) || ( otherStartY > endY ) ) )
    {
        return edgeRight;
    }

    return edgeNone;
}

Rectangle Rectangle::clipTo( const Rectangle& other ) const
{
    int _x( originX() );
    int _y( originY() );
    int _height( height() );
    int _width( width() );

    int xlDelta( other.originX() - _x );
    int xrDelta( ( _x + _width ) - ( other.originX() + other.width() ) );
    int ytDelta( other.originY() - _y );
    int ybDelta( ( _y + _height ) - ( other.originY() + other.height() ) );

    if( xlDelta > 0 )
    {
        _width -= xlDelta;
        _x = other.originX();
    }
    if( xrDelta > 0 )
    {
        _width -= xrDelta;
    }
    if( ytDelta > 0 )
    {
        _height -= ytDelta;
        _y = other.originY();
    }
    if( ybDelta > 0 )
    {
        _height -= ybDelta;
    }

    return Rectangle( _x, _y, _height, _width );
}

int Rectangle::originX() const
{
    return m_upperLeft.m_x;
}

int Rectangle::originY() const
{
    return m_upperLeft.m_y;
}

int Rectangle::height() const
{
    return( m_bottomRight.m_y - m_upperLeft.m_y + 1 );
}

int Rectangle::width() const
{
    return( m_bottomRight.m_x - m_upperLeft.m_x + 1 );
}

void Rectangle::translate( const Point& origin )
{
    int distX( origin.m_x - m_upperLeft.m_x );
    int distY( origin.m_y - m_upperLeft.m_y );
    m_upperLeft = origin;
    m_bottomRight.m_x += distX;
    m_bottomRight.m_y += distY;
}

String Rectangle::dump() const
{
    LiteStream stream;
    stream << originX() << "," << originY() << " " << height() << "," << width();
    return stream.str();
}

} // namespace Agape
