#ifndef AGAPE_LINDA2_TUPLE_ROUTER_H
#define AGAPE_LINDA2_TUPLE_ROUTER_H

#include "Collections.h"
#include "Runnable.h"
#include "String.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

namespace Linda2
{

class Actor;
class Tuple;
class TupleDispatcher;
class TupleFilter;
class TupleRoute;
class TupleRoutingCriteria;

class TupleRouter
{
public:
    TupleRouter( TupleDispatcher& tupleDispatcher,
                 const String& routerName,
                 Timers::Factory& timerFactory );

    // FIXME: Should use const reference, but then Carlo does not handle
    // const tuples correctly yet (see comment in Actor.h).
    bool route( Tuple& tuple );

    void addRoute( TupleRoute* route, bool defaultRoute = true );
    void removeRoute( TupleRoute* route );

    void setMyID( const String& id );
    const String& myID() const;

    void run();

    static const String& tupleType( const Tuple& tuple );

    static const String& sourceActor( const Tuple& tuple );
    static const String& destinationActor( const Tuple& tuple );
    
    static const String& sourceID( const Tuple& tuple );
    static const String& destinationID( const Tuple& tuple );

    static void setTupleType( Tuple& tuple, const String& type );
    
    static void setSourceActor( Tuple& tuple, const String& sourceActor );
    static void setDestinationActor( Tuple& tuple, const String& destinationActor );

    static void setSourceID( Tuple& tuple, const String& id );
    static void setDestinationID( Tuple& tuple, const String& id );

    // Pass-through to TupleDispatcher.
    void registerActor( Actor* actor );
    void deregisterActor( Actor* actor );

    void registerMonitor( Actor* actor );
    void deregisterMonitor( Actor* actor );

    // Pass-through to default route.
    void sendAddRoutingCriteriaRequest( const TupleRoutingCriteria& routingCriteria );
    void sendRemoveRoutingCriteriaRequest( const TupleRoutingCriteria& routingCriteria );

    void setTupleFilter( TupleFilter* tupleFilter );

    bool routeError();

    // Unlike Tuple::dump(), which performs a complete and recursive dump, this
    // produces a colourised one-liner for logging from within TupleRouter and
    // for Linda2Strategy.
    String transferDump( Tuple& tuple );

private:
    void handleRoutingRequest( const Tuple& tuple, TupleRoute* route );

    bool permitIn( const Tuple& tuple );
    bool permitInDefault( const Tuple& tuple );
    bool permitForward( const Tuple& tuple );
    bool permitForwardDefault( const Tuple& tuple );
    bool permitOut( const Tuple& tuple );
    bool permitOutDefault( const Tuple& tuple );

    TupleDispatcher& m_tupleDispatcher;
    String m_routerName;
    Timers::Factory& m_timerFactory;

    Vector< TupleRoute* > m_tupleRoutes;
    TupleRoute* m_defaultRoute;

    TupleFilter* m_tupleFilter;

    String m_myID;

    bool m_routeError;
};

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_TUPLE_ROUTER_H
