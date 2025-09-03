#ifndef AGAPE_LINDA2_TUPLE_DISPATCHER_H
#define AGAPE_LINDA2_TUPLE_DISPATCHER_H

#include "Value.h"

#include "Collections.h"
#include "String.h"

namespace Agape
{

namespace Linda2
{

class Actor;
class Tuple;

class TupleDispatcher
{
public:
    TupleDispatcher();

    void registerActor( Actor* actor );
    void deregisterActor( Actor* actor );

    void registerMonitor( Actor* actor );
    void deregisterMonitor( Actor* actor );

    bool dispatch( Tuple& tuple );

private:
    List< Actor* > m_actors;
    Actor* m_monitor;
};

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_TUPLE_DISPATCHER_H
