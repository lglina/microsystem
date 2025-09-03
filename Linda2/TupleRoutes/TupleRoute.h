#ifndef AGAPE_LINDA2_TUPLE_ROUTE_H
#define AGAPE_LINDA2_TUPLE_ROUTE_H

#include "Collections.h"
#include "Runnable.h"
#include "String.h"
#include "TupleRoutingCriteria.h"

namespace Agape
{

namespace Linda2
{

namespace TupleRoutes
{
class Chooser;
} // namespace TupleRoutes

class Tuple;

class TupleRoute
{
    friend TupleRoutes::Chooser;

public:
    TupleRoute( const String& routeName );
    virtual ~TupleRoute() {}

    virtual bool haveIncoming() = 0;
    virtual bool receiveTuple( Tuple& tuple ) = 0; // true = received. false = nothing to receive or error (caller should check error()).
    bool sendTuple( const Tuple& tuple, bool unconditional = false ); // true = success. false = failure.

    void sendAddRoutingCriteriaRequest( const TupleRoutingCriteria& routingCriteria );
    void sendRemoveRoutingCriteriaRequest( const TupleRoutingCriteria& routingCriteria );

    void addRoutingCriteria( const TupleRoutingCriteria& routingCriteria );
    void removeRoutingCriteria( const TupleRoutingCriteria& routingCriteria );

    const String& name();

    virtual void run() = 0;

    virtual bool error() const = 0;

protected:
    bool canRoute( const Tuple& tuple ) const;

    String m_routeName;

private:
    virtual bool _sendTuple( const Tuple& tuple ) = 0;

    Vector< TupleRoutingCriteria > m_tupleRoutingCriteria;
};

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_TUPLE_ROUTE_H
