#ifndef AGAPE_TUPLE_FILTERS_STRATUS_H
#define AGAPE_TUPLE_FILTERS_STRATUS_H

#include "TupleFilters/TupleFilter.h"

namespace Agape
{

namespace Stratus
{
class Authenticator;
} // namespace Stratus

using namespace Stratus;

namespace Linda2
{

class TupleRouter;

namespace TupleFilters
{

class Stratus : public TupleFilter
{
public:
    Stratus( Authenticator& authenticator );

    virtual bool permitIn( const Tuple& tuple );
    virtual bool permitInDefault( const Tuple& tuple );
    virtual bool permitForward( const Tuple& tuple );
    virtual bool permitForwardDefault( const Tuple& tuple );
    virtual bool permitOut( const Tuple& tuple );
    virtual bool permitOutDefault( const Tuple& tuple );

private:
    Authenticator& m_authenticator;
};

} // namespace TupleFilters

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_TUPLE_FILTERS_STRATUS_H
