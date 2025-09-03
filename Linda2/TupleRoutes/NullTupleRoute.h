#ifndef AGAPE_LINDA2_TUPLE_ROUTES_NULL_H
#define AGAPE_LINDA2_TUPLE_ROUTES_NULL_H

#include "String.h"
#include "TupleRoute.h"

namespace Agape
{

namespace Linda2
{

class Tuple;

namespace TupleRoutes
{

class Null : public TupleRoute
{
public:
    Null( const String& routeName );

    virtual bool haveIncoming();
    virtual bool receiveTuple( Tuple& tuple );
    virtual void sendTuple( const Tuple& tuple );

    virtual void run();

    virtual bool error() const;

private:
    virtual bool _sendTuple( const Tuple& tuple );
};

}

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_TUPLE_ROUTES_NULL_H
