#ifndef AGAPE_CARTESIAN_H
#define AGAPE_CARTESIAN_H

#include "String.h"

namespace Agape
{

class Point
{
public:
    Point();
    Point( int x, int y );

    int m_x;
    int m_y;
};

class Rectangle
{
public:
    enum Edge
    {
        edgeNone,
        edgeTop,
        edgeBottom,
        edgeLeft,
        edgeRight
    };

    Rectangle();
    Rectangle( const Point& upperLeft, const Point& bottomRight );
    Rectangle( const Point& origin, int height, int width );
    Rectangle( int x, int y, int height, int width );

    bool intersects( const Rectangle& other ) const;
    Rectangle findIntersection( const Rectangle& other ) const;

    bool pointIntersects( const Point& point, int rectExpansion = 0 ) const;

    Rectangle findUnionBoundingBox( const Rectangle& other ) const;

    enum Edge edgeTouches( const Rectangle& other ) const;

    Rectangle clipTo( const Rectangle& other ) const;

    int originX() const;
    int originY() const;
    int height() const;
    int width() const;

    void translate( const Point& origin );

    String dump() const;

    Point m_upperLeft;
    Point m_bottomRight;
};

} // namespace Agape

#endif // AGAPE_CARTESIAN_H
