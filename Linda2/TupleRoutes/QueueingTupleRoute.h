#ifndef AGAPE_LINDA2_TUPLE_ROUTES_QUEUEING_H
#define AGAPE_LINDA2_TUPLE_ROUTES_QUEUEING_H

#include "Collections.h"
#include "Runnable.h"
#include "String.h"
#include "TupleRoute.h"
#include "TupleRoutingCriteria.h"

#include <condition_variable>
#include <mutex>

namespace Agape
{

namespace Linda2
{

class Tuple;

namespace TupleRoutes
{

class Queueing : public TupleRoute
{
public:
    Queueing( const String& routeName );
    ~Queueing();

    virtual bool haveIncoming();
    virtual bool receiveTuple( Tuple& tuple );

    virtual void run();
    void stop();

    virtual bool error() const;

    void setPartner( Queueing* partner );

    void waitIncoming();
    void enqueue( const Tuple& tuple );

private:
    virtual bool _sendTuple( const Tuple& tuple );

    Deque< Tuple > m_incomingQueue;
    std::mutex m_mutex;
    std::condition_variable m_incomingPending;

    Queueing* m_partner;

    bool m_stop;
};

} // namespace TupleRoutes

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_TUPLE_ROUTES_QUEUEING_H
