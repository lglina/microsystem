#include "String.h"
#include "StringConstants.h"
#include "UniverseStats.h"
#include "Value.h"

namespace Agape
{

namespace World
{

UniverseStats::UniverseStats() :
  m_items( 0 )
{
}

void UniverseStats::toValue( Value& value ) const
{
    value[_items] = m_items;
}

UniverseStats UniverseStats::fromValue( const Value& value )
{
    UniverseStats stats;

    stats.m_items = value[_items];

    return stats;
}

} // namespace World

} // namespace Agape
