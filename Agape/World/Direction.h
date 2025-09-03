#ifndef AGAPE_WORLD_DIRECTION_H
#define AGAPE_WORLD_DIRECTION_H

namespace Agape
{

class Value;

namespace World
{

class Direction
{
public:
    enum _Direction
    {
        up,
        down,
        left,
        right,
        none
    };
    
    Direction();
    Direction( enum _Direction direction );

    void toValue( Value& value ) const;
    static Direction fromValue( const Value& value );

    enum _Direction m_direction;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_WORLD_DIRECTION_H
