#include "Loggers/Logger.h"
#include "TelegramLoaders/TelegramLoader.h"
#include "Timers/Factories/TimerFactory.h"
#include "World/Telegram.h"
#include "Collections.h"
#include "Linda2TelegramLoader.h"
#include "Promise.h"
#include "String.h"
#include "StringConstants.h"
#include "TupleRouter.h"
#include "Tuple.h"

using namespace Agape::Linda2;
using namespace Agape::World;

namespace Agape
{

namespace TelegramLoaders
{

// FIXME: There's a lot of repetition with the Linda2 loaders - sending a
// request and receiving one or more items (which are deserialised from tuples)
// is a common pattern and can probably be broken out to a separate class,
// perhaps templated on the desired sent/received object type.
Linda2::Linda2( const String& recipientSnowflake,
                TupleRouter& tupleRouter,
                Timers::Factory& timerFactory ) :
  TelegramLoader( recipientSnowflake ),
  Native( "TelegramLoader" ),
  m_tupleRouter( tupleRouter ),
  m_timerFactory( timerFactory ),
  m_currentItem( 0 ),
  m_totalItems( -1 ),
  m_telegrams( nullptr )
{
    LOG_DEBUG( "Linda2TelegramLoader: Created" );
    m_tupleRouter.registerActor( this );
}

Linda2::~Linda2()
{
    LOG_DEBUG( "Linda2TelegramLoader: Destroyed" );
    m_tupleRouter.deregisterActor( this );
}

bool Linda2::load( Vector< Telegram >& telegrams )
{
    bool success( false );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _TelegramLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _TelegramLoadRequest );
    tuple[_recipientSnowflake] = m_recipientSnowflake;

    m_telegrams = &telegrams;
    m_currentItem = 0;
    m_totalItems = -1;

    LOG_DEBUG( "Linda2TelegramLoader: Sending TelegramLoadRequest" );
    m_telegramLoadResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_telegramLoadResponse.getFuture().get();

    m_telegrams = nullptr;

    return success;
}

bool Linda2::loadSent( Vector< Telegram >& telegrams )
{
    bool success( false );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _TelegramLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _TelegramLoadSentRequest );
    tuple[_recipientSnowflake] = m_recipientSnowflake;

    m_telegrams = &telegrams;
    m_currentItem = 0;
    m_totalItems = -1;

    LOG_DEBUG( "Linda2TelegramLoader: Sending TelegramLoadSentRequest" );
    m_telegramLoadSentResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_telegramLoadSentResponse.getFuture().get();

    m_telegrams = nullptr;

    return success;
}

bool Linda2::send( const Telegram& telegram )
{
    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _TelegramLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _TelegramSendRequest );
    tuple[_recipientSnowflake] = m_recipientSnowflake;
    telegram.toValue( tuple[_telegram] );

    LOG_DEBUG( "Linda2TelegramLoader: Sending TelegramSendRequest" );
    m_telegramSendResponse = Promise( &m_tupleRouter, &m_timerFactory );
    if( m_tupleRouter.route( tuple ) ) return m_telegramSendResponse.getFuture().get();
    return false;
}

bool Linda2::markRead( const Telegram& telegram )
{
    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _TelegramLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _TelegramMarkReadRequest );
    tuple[_recipientSnowflake] = m_recipientSnowflake;
    telegram.toValue( tuple[_telegram] );

    LOG_DEBUG( "Linda2TelegramLoader: Sending TelegramMarkReadRequest" );
    m_telegramMarkReadResponse = Promise( &m_tupleRouter, &m_timerFactory );
    if( m_tupleRouter.route( tuple ) ) return m_telegramMarkReadResponse.getFuture().get();
    return false;
}

bool Linda2::erase( const Telegram& telegram )
{
    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _TelegramLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _TelegramEraseRequest );
    tuple[_recipientSnowflake] = m_recipientSnowflake;
    telegram.toValue( tuple[_telegram] );

    LOG_DEBUG( "Linda2TelegramLoader: Sending TelegramEraseRequest" );
    m_telegramEraseResponse = Promise( &m_tupleRouter, &m_timerFactory );
    if( m_tupleRouter.route( tuple ) ) return m_telegramEraseResponse.getFuture().get();
    return false;
}

bool Linda2::unread( Map< String, int >& numUnread, bool allDevices )
{
    numUnread.clear();

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _TelegramLoader );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _TelegramUnreadRequest );
    tuple[_recipientSnowflake] = m_recipientSnowflake;
    tuple[_allDevices] = allDevices ? 1 : 0;

    LOG_DEBUG( "Linda2TelegramLoader: Sending TelegramUnreadRequest" );
    m_telegramUnreadResponse = Promise( &m_tupleRouter, &m_timerFactory );
    if( m_tupleRouter.route( tuple ) )
    {
        Value numUnreadValue;
        if( m_telegramUnreadResponse.getFuture().get( numUnreadValue ) )
        {
            ConstMapIterator it( numUnreadValue.mapBegin() );
            for( ; it != numUnreadValue.mapEnd(); ++it )
            {
                numUnread[it->first] = (int)*( it->second );
            }
            return true;
        }
    }

    return false;
}

bool Linda2::accept( Tuple& tuple )
{
    bool handled( false );

    if( TupleRouter::tupleType( tuple ) == _TelegramLoadSummary )
    {
        LOG_DEBUG( "Linda2TelegramLoader: Received telegram load summary" );
        m_totalItems = tuple[_totalItems];

        if( m_currentItem == m_totalItems )
        {
            m_telegramLoadResponse.set( tuple[_success] );
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _TelegramLoadResponse ) && m_telegrams )
    {
        LOG_DEBUG( "Linda2TelegramLoader: Received telegram metadata" );
        Telegram telegram( Telegram::fromValue( tuple[_telegram] ) );
        m_telegrams->push_back( telegram );
        ++m_currentItem;

        if( m_currentItem == m_totalItems )
        {
            m_telegramLoadResponse.set();
        }

        handled = true;
    }
    if( TupleRouter::tupleType( tuple ) == _TelegramLoadSentSummary )
    {
        LOG_DEBUG( "Linda2TelegramLoader: Received telegram load sent summary" );
        m_totalItems = tuple[_totalItems];

        if( m_currentItem == m_totalItems )
        {
            m_telegramLoadSentResponse.set( tuple[_success] );
        }

        handled = true;
    }
    else if( ( TupleRouter::tupleType( tuple ) == _TelegramLoadSentResponse ) && m_telegrams )
    {
        LOG_DEBUG( "Linda2TelegramLoader: Received sent telegram metadata" );
        Telegram telegram( Telegram::fromValue( tuple[_telegram] ) );
        m_telegrams->push_back( telegram );
        ++m_currentItem;

        if( m_currentItem == m_totalItems )
        {
            m_telegramLoadSentResponse.set();
        }

        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _TelegramSendResponse )
    {
        LOG_DEBUG( "Linda2TelegramLoader: Received telegram send response" );
        m_telegramSendResponse.set( tuple[_success] );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _TelegramMarkReadResponse )
    {
        LOG_DEBUG( "Linda2TelegramLoader: Received telegram mark read response" );
        m_telegramMarkReadResponse.set( tuple[_success] );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _TelegramEraseResponse )
    {
        LOG_DEBUG( "Linda2TelegramLoader: Received telegram erase response" );
        m_telegramEraseResponse.set( tuple[_success] );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _TelegramUnreadResponse )
    {
        LOG_DEBUG( "Linda2TelegramLoader: Received telegram unread response" );
        m_telegramUnreadResponse.set( tuple[_success], tuple[_numUnread] );
        handled = true;
    }

    return handled;
}

} // namespace TelegramLoaders

} // namespace Agape
