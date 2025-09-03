#include "String.h"
#include "StringConstants.h"
#include "Value.h"
#include "WorldSummary.h"

namespace Agape
{

namespace World
{

Summary::Summary() :
  m_items( 0 )
{
}

void Summary::toValue( Value& value ) const
{
    value[_worldID] = m_worldID;
    value[_items] = m_items;
    value[_users] = m_users;
}

Summary Summary::fromValue( const Value& value )
{
    Summary summary;

    summary.m_worldID = value[_worldID];
    summary.m_items = value[_items];
    summary.m_users = value[_users];

    return summary;
}

} // namespace World

} // namespace Agape
