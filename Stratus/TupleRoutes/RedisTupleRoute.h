#ifndef AGAPE_LINDA2_TUPLE_ROUTES_REDIS_H
#define AGAPE_LINDA2_TUPLE_ROUTES_REDIS_H

#include "TupleRoutes/TupleRoute.h"
#include "String.h"

#include <hiredis/hiredis.h>

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

#include <event.h>

namespace Agape
{

namespace Linda2
{

class Tuple;

namespace TupleRoutes
{

class Redis : public TupleRoute
{
public:
    Redis( const String& routeName );
    ~Redis();

    void connect();

    virtual bool haveIncoming();
    virtual bool receiveTuple( Tuple& tuple );

    virtual void run();

    virtual bool error() const;

    void waitIncoming();
    void stop();

private:
    virtual bool _sendTuple( const Tuple& tuple );

    void dispatchEvents();
    static void onReply( redisAsyncContext* context, void* reply, void* privateData );
    void handleReply( const redisReply* reply );
    static void onSubscribe( evutil_socket_t fd, short events, void* arg );
    void handleSubscribe();
    static void onUnsubscribe( evutil_socket_t fd, short events, void* arg );
    void handleUnsubscribe();
    static void onPublish( evutil_socket_t fd, short events, void* arg );
    void handlePublish();

    void subscribe( const TupleRoutingCriteria& routingCriteria );
    void unsubscribe( const TupleRoutingCriteria& routingCriteria );
    void unsubscribe( const String& worldID );
    bool publish( const Tuple& tuple );

    struct event_base* m_eventBase;
    redisAsyncContext* m_publishContext;
    redisAsyncContext* m_subscribeContext;

    bool m_stopping;

    std::unique_ptr< std::thread > m_eventThread;

    std::mutex m_queuesMutex;

    std::deque< Tuple > m_incomingQueue;
    std::condition_variable m_incomingPending;

    std::deque< String > m_pendingSubscribe;
    std::deque< String > m_pendingUnsubscribe;
    std::deque< std::pair< String, String > > m_pendingPublish;

    std::deque< String > m_subscribedWorlds;
};

}

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_TUPLE_ROUTES_REDIS_H
