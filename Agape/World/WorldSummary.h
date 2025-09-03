#ifndef AGAPE_WORLD_SUMMARY_H
#define AGAPE_WORLD_SUMMARY_H

#include "String.h"

namespace Agape
{

class Value;

namespace World
{

class Summary
{
public:
    Summary();

    void toValue( Value& value ) const;
    static Summary fromValue( const Value& value );

    String m_worldID;
    int m_items;
    int m_users;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_WORLD_SUMMARY_H
