#ifndef AGAPE_WORLD_COORDINATES_H
#define AGAPE_WORLD_COORDINATES_H

#include "String.h"
#include "StringConstants.h"

namespace Agape
{

namespace Linda2
{
    class TupleRoutingCriteria;
} // namespace Linda2

using namespace Linda2;

class Value;

namespace World
{

class Coordinates
{
public:
    Coordinates();
    Coordinates( const String& worldID );
    Coordinates( const String& worldID, int x, int y );

    bool operator==( const Coordinates& other ) const;
    bool operator!=( const Coordinates& other ) const;

    void toValue( Value& value, bool allScenes = false ) const;
    static Coordinates fromValue( const Value& value );

    void toRoutingCriteria( TupleRoutingCriteria& tupleRoutingCriteria,
                            const char* valueName = _coordinates,
                            bool allScenes = false ) const;

    String m_worldID;
    int m_x;
    int m_y;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_WORLD_COORDINATES_H
