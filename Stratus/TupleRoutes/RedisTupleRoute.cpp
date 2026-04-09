#include "Loggers/Logger.h"
#include "TupleRoutes/TupleRoute.h"
#include "Utils/LiteStream.h"
#include "RedisTupleRoute.h"
#include "String.h"
#include "StringConstants.h"
#include "StringSerialiser.h"
#include "Tuple.h"
#include "TupleRouter.h"

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

#include <event.h>
#include <event2/thread.h>
#include <unistd.h>

namespace Agape
{

namespace Linda2
{

namespace TupleRoutes
{

Redis::Redis( const String& routeName ) :
  m_eventBase( nullptr ),
  m_publishContext( nullptr ),
  m_subscribeContext( nullptr ),
  m_stopping( false ),
  TupleRoute( routeName )
{
}

Redis::~Redis()
{
    Deque< String >::iterator it( m_subscribedWorlds.begin() );
    for( ; it != m_subscribedWorlds.end(); ++it )
    {
        unsubscribe( *it );
    }

    m_stopping = true;
    if( m_eventBase && m_eventThread )
    {
        event_base_loopexit( m_eventBase, NULL );
        m_eventThread->join();
    }

    if( m_publishContext ) redisAsyncFree( m_publishContext );
    if( m_subscribeContext ) redisAsyncFree( m_subscribeContext );
}

void Redis::connect()
{
    bool success( true );

    evthread_use_pthreads();

    m_eventBase = event_base_new();

    // FIXME: We really need connection pooling here, but it isn't supported
    // in hiredis?
    m_publishContext = redisAsyncConnect( "127.0.0.1", 6379 );

    if( m_publishContext->err )
    {
        LiteStream stream;
        stream << "RedisTupleRoute: Error creating async context: "
               << m_publishContext->errstr;
        LOG_DEBUG( stream.str() );
        redisAsyncFree( m_publishContext );
        m_publishContext = nullptr;
        success = false;
    }

    if( success )
    {
        m_subscribeContext = redisAsyncConnect( "127.0.0.1", 6379 );

        if( m_subscribeContext->err )
        {
            LiteStream stream;
            stream << "RedisTupleRoute: Error creating async context: "
                << m_subscribeContext->errstr;
            LOG_DEBUG( stream.str() );
            redisAsyncFree( m_subscribeContext );
            m_subscribeContext = nullptr;
            success = false;
        }
    }

    if( success )
    {
        redisLibeventAttach( m_publishContext, m_eventBase );
        redisLibeventAttach( m_subscribeContext, m_eventBase );
        m_eventThread.reset( new std::thread( std::bind( &Redis::dispatchEvents, this ) ) );

        std::unique_lock< std::mutex > lock( m_queuesMutex );
        m_pendingSubscribe.push_back( _Clock ); // Subscribe to the master clock

        struct event* ev( event_new( m_eventBase, -1, EV_READ, &Redis::onSubscribe, (void*)this ) );
        event_add( ev, NULL );
        event_active( ev, EV_WRITE, 0 );
    }
}

bool Redis::haveIncoming()
{
    std::unique_lock< std::mutex > lock( m_queuesMutex );
    return !m_incomingQueue.empty();
}

bool Redis::receiveTuple( Tuple& tuple )
{
    if( haveIncoming() )
    {
        std::unique_lock< std::mutex > lock( m_queuesMutex );
        tuple = m_incomingQueue.front();
        m_incomingQueue.pop_front();
        return true;
    }

    return false;
}

void Redis::run()
{
    // NOP
}

bool Redis::error() const
{
    // FIXME: Stub.
    return false;
}

void Redis::waitIncoming()
{
    std::unique_lock< std::mutex > lock( m_queuesMutex );
    if( m_incomingQueue.empty() )
    {
        m_incomingPending.wait( lock );
    }
}

void Redis::stop()
{
    m_incomingPending.notify_all();
}

bool Redis::_sendTuple( const Tuple& tuple )
{
    // Publish a message using worldID as the topic, UNLESS
    // this is a routing criteria message, in which case drop it here and
    // subscribe to the worldID within the routing criteria
    if( TupleRouter::tupleType( tuple ) == _RoutingCriteria )
    {
        TupleRoutingCriteria routingCriteria( TupleRoutingCriteria::fromTuple( tuple ) );
        if( tuple[_action] == String(_add) )
        {
            subscribe( routingCriteria );
        }
        else if( tuple[_action] == String(_remove) )
        {
            unsubscribe( routingCriteria );
        }
    }

    return publish( tuple );
}

void Redis::dispatchEvents()
{
    while( !m_stopping )
    {
        event_base_dispatch( m_eventBase );
    }
}

void Redis::onReply( redisAsyncContext* context, void* reply, void* privateData )
{
    redisReply* r = (redisReply*)reply;
    if( reply && privateData )
    {
        Redis* instance = (Redis*)privateData;
        instance->handleReply( r );
    }
}

void Redis::handleReply( const redisReply* reply )
{
    if( reply &&
        ( reply->type == REDIS_REPLY_ARRAY ) &&
        ( reply->elements == 3 ) &&
        ( reply->element[2]->str ) )
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "RedisTupleRoute: Handling reply" );
#endif
        StringSerialiser serialiser;
        serialiser.m_data = String( reply->element[2]->str, reply->element[2]->len );
        Tuple tuple;
        Tuple::fromReadableWritable( serialiser, tuple );

        std::unique_lock< std::mutex > lock( m_queuesMutex );
        m_incomingQueue.push_back( tuple );
        m_incomingPending.notify_all();
#ifdef LOG_TUPLES
        LOG_DEBUG( "RedisTupleRoute: Notified reply" );
#endif
    }
    else
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "RedisTupleRoute: Invalid reply" );
#endif
    }
}

void Redis::onSubscribe( evutil_socket_t fd, short events, void* arg )
{
    if( arg )
    {
        Redis* instance = (Redis*)arg;
        instance->handleSubscribe();
    }
}

void Redis::handleSubscribe()
{
    std::unique_lock< std::mutex > lock( m_queuesMutex );

    while( !m_pendingSubscribe.empty() )
    {
        String topic( m_pendingSubscribe.front() );
        m_pendingSubscribe.pop_front();

        if( redisAsyncCommand( m_subscribeContext,
                               &Redis::onReply,
                               (void*)this,
                               "SUBSCRIBE %s", topic.c_str() ) == REDIS_OK )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "RedisTupleRoute: Subscribe done." );
#endif
        }
        else
        {
            LOG_DEBUG( "RedisTupleRoute: Error subscribing to topic." );
        }
    }
}

void Redis::onUnsubscribe( evutil_socket_t fd, short events, void* arg )
{
    if( arg )
    {
        Redis* instance = (Redis*)arg;
        instance->handleUnsubscribe();
    }
}

void Redis::handleUnsubscribe()
{
    std::unique_lock< std::mutex > lock( m_queuesMutex );

    while( !m_pendingUnsubscribe.empty() )
    {
        String topic( m_pendingUnsubscribe.front() );
        m_pendingUnsubscribe.pop_front();

        if( redisAsyncCommand( m_subscribeContext,
                               &Redis::onReply,
                               (void*)this,
                               "UNSUBSCRIBE %s", topic.c_str() ) == REDIS_OK )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "RedisTupleRoute: Unsubscribe done." );
#endif
        }
        else
        {
            LOG_DEBUG( "RedisTupleRoute: Error unsubscribing from topic." );
        }
    }
}

void Redis::onPublish( evutil_socket_t fd, short events, void* arg )
{
    if( arg )
    {
        Redis* instance = (Redis*)arg;
        instance->handlePublish();
    }
}

void Redis::handlePublish()
{
    std::unique_lock< std::mutex > lock( m_queuesMutex );

    while( !m_pendingPublish.empty() )
    {
        String topic( m_pendingPublish.front().first );
        String message( m_pendingPublish.front().second );
        m_pendingPublish.pop_front();

        if( redisAsyncCommand( m_publishContext,
                               &Redis::onReply,
                               (void*)this,
                               "PUBLISH %s %b", topic.c_str(), message.data(), message.length() ) == REDIS_OK )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "RedisTupleRoute: Publish done." );
#endif
        }
        else
        {
            LOG_DEBUG( "RedisTupleRoute: Error publishing to topic." );
        }
    }
}

void Redis::subscribe( const TupleRoutingCriteria& routingCriteria )
{
    if( routingCriteria.m_values.hasValue( _coordinates ) &&
        routingCriteria.m_values[_coordinates].hasValue( _worldID ) )
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "RedisTupleRoute: Subscribing" );
#endif
        String worldID = routingCriteria.m_values[_coordinates][_worldID];

        Deque< String >::iterator it( m_subscribedWorlds.begin() );
        for( ; it != m_subscribedWorlds.end(); ++it )
        {
            if( *it == worldID )
            {
                return; // Already subscribed.
            }
        }

        m_subscribedWorlds.push_back( worldID );

        std::unique_lock< std::mutex > lock( m_queuesMutex );
        m_pendingSubscribe.push_back( worldID );

        struct event* ev( event_new( m_eventBase, -1, EV_READ, &Redis::onSubscribe, (void*)this ) );
        event_add( ev, NULL );
        event_active( ev, EV_WRITE, 0 );
    }
    else
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "RedisTupleRoute: RoutingCriteria has no coordinates. Not subscribing via Redis." );
#endif
    }
}

void Redis::unsubscribe( const TupleRoutingCriteria& routingCriteria )
{
    if( routingCriteria.m_values.hasValue( _coordinates ) &&
        routingCriteria.m_values[_coordinates].hasValue( _worldID ) )
    {
        String worldID = routingCriteria.m_values[_coordinates][_worldID];

        Deque< String >::iterator it( m_subscribedWorlds.begin() );
        for( ; it != m_subscribedWorlds.end(); ++it )
        {
            if( *it == worldID )
            {
                m_subscribedWorlds.erase( it );
                break;
            }
        }

        if( it != m_subscribedWorlds.end() )
        {
            unsubscribe( worldID );
        }
    }
    else
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "RedisTupleRoute: RoutingCriteria has no coordinates. Not unsubscribing via Redis." );
#endif
    }
}

void Redis::unsubscribe( const String& worldID )
{
    std::unique_lock< std::mutex > lock( m_queuesMutex );
    m_pendingUnsubscribe.push_back( worldID );

    struct event* ev( event_new( m_eventBase, -1, EV_READ, &Redis::onUnsubscribe, (void*)this ) );
    event_add( ev, NULL );
    event_active( ev, EV_WRITE, 0 );
}

bool Redis::publish( const Tuple& tuple )
{
    if( tuple.hasValue( _coordinates ) &&
        tuple[_coordinates].hasValue( _worldID ) )
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "RedisTupleRoute: Publishing." );
#endif

        String worldID = tuple[_coordinates][_worldID];

        StringSerialiser serialiser;
        tuple.toReadableWritable( serialiser );

        std::unique_lock< std::mutex > lock( m_queuesMutex );
        m_pendingPublish.push_back( std::make_pair( worldID, serialiser.m_data ) );

        struct event* ev( event_new( m_eventBase, -1, EV_READ, &Redis::onPublish, (void*)this ) );
        event_add( ev, NULL );
        event_active( ev, EV_WRITE, 0 );
    }
    else
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "RedisTupleRoute: Tuple has no coordinates. Not publishing via Redis." );
#endif
        return false;
    }

    return true;
}

}

} // namespace Linda2

} // namespace Agape
