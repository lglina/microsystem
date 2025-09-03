#ifndef AGAPE_WORLD_UNIVERSE_STATS_H
#define AGAPE_WORLD_UNIVERSE_STATS_H

#include "String.h"

namespace Agape
{

class Value;

namespace World
{

class UniverseStats
{
public:
    UniverseStats();

    void toValue( Value& value ) const;
    static UniverseStats fromValue( const Value& value );

    int m_items;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_WORLD_UNIVERSE_STATS_H
