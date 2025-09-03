#include "Loggers/Logger.h"
#include "TelegramLoaders/Factories/TelegramLoadersFactory.h"
#include "TelegramLoaders/TelegramLoader.h"
#include "World/Telegram.h"
#include "Collections.h"
#include "Linda2TelegramLoaderResponder.h"
#include "String.h"
#include "StringConstants.h"
#include "TupleRouter.h"
#include "Tuple.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace TelegramLoaders
{

Linda2Responder::Linda2Responder( TupleRouter& tupleRouter, TelegramLoaders::Factory& telegramLoaderFactory ) :
  m_tupleRouter( tupleRouter ),
  m_telegramLoaderFactory( telegramLoaderFactory ),
  Native( "TelegramLoaderResponder" )
{
    m_tupleRouter.registerActor( this );
}

Linda2Responder::~Linda2Responder()
{
    m_tupleRouter.deregisterActor( this );
}

bool Linda2Responder::accept( Tuple& tuple )
{
    bool handled( false );

    if( TupleRouter::tupleType( tuple ) == _TelegramLoadRequest )
    {
        load( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _TelegramLoadSentRequest )
    {
        loadSent( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _TelegramSendRequest )
    {
        send( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _TelegramMarkReadRequest )
    {
        markRead( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _TelegramEraseRequest )
    {
        erase( tuple );
        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _TelegramUnreadRequest )
    {
        unread( tuple );
        handled = true;
    }

    return handled;
}

void Linda2Responder::reset()
{
}

void Linda2Responder::load( const Tuple& tuple )
{
    String recipientSnowflake = tuple[_recipientSnowflake];
    TelegramLoader* telegramLoader( m_telegramLoaderFactory.makeLoader( recipientSnowflake ) );

    LiteStream stream;
    stream << "Linda2TelegramLoaderResponder: Received TelegramLoadRequest for "
           << recipientSnowflake;
    LOG_DEBUG( stream.str() );

    Vector< Telegram > telegrams;
    bool success( telegramLoader->load( telegrams ) );

    LOG_DEBUG( "Linda2TelegramLoaderResponder: Sending TelegramLoadSummary" );
    Tuple response;
    TupleRouter::setSourceActor( response, _TelegramLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _TelegramLoadSummary );
    response[_totalItems] = success ? (int)telegrams.size() : 0;
    response[_success] = success;
    m_tupleRouter.route( response );

    Vector< Telegram >::const_iterator it( telegrams.begin() );
    for( ; success && ( it != telegrams.end() ); ++it )
    {
        LOG_DEBUG( "Linda2TelegramLoaderResponder: Sending TelegramLoadResponse" );
        Tuple telegramTuple;
        TupleRouter::setSourceActor( telegramTuple, _TelegramLoaderResponder );
        TupleRouter::setSourceID( telegramTuple, m_tupleRouter.myID() );
        TupleRouter::setDestinationID( telegramTuple, TupleRouter::sourceID( tuple ) );
        TupleRouter::setTupleType( telegramTuple, _TelegramLoadResponse );
        it->toValue( telegramTuple[_telegram] );
        m_tupleRouter.route( telegramTuple );
    }

    delete( telegramLoader );
}

void Linda2Responder::loadSent( const Tuple& tuple )
{
    String recipientSnowflake = tuple[_recipientSnowflake];
    TelegramLoader* telegramLoader( m_telegramLoaderFactory.makeLoader( recipientSnowflake ) );

    LiteStream stream;
    stream << "Linda2TelegramLoaderResponder: Received TelegramLoadSentRequest for "
           << recipientSnowflake;
    LOG_DEBUG( stream.str() );

    Vector< Telegram > telegrams;
    bool success( telegramLoader->loadSent( telegrams ) );

    LOG_DEBUG( "Linda2TelegramLoaderResponder: Sending TelegramLoadSentSummary" );
    Tuple response;
    TupleRouter::setSourceActor( response, _TelegramLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _TelegramLoadSentSummary );
    response[_totalItems] = success ? (int)telegrams.size() : 0;
    response[_success] = success;
    m_tupleRouter.route( response );

    Vector< Telegram >::const_iterator it( telegrams.begin() );
    for( ; success && ( it != telegrams.end() ); ++it )
    {
        LOG_DEBUG( "Linda2TelegramLoaderResponder: Sending TelegramLoadSentResponse" );
        Tuple telegramTuple;
        TupleRouter::setSourceActor( telegramTuple, _TelegramLoaderResponder );
        TupleRouter::setSourceID( telegramTuple, m_tupleRouter.myID() );
        TupleRouter::setDestinationID( telegramTuple, TupleRouter::sourceID( tuple ) );
        TupleRouter::setTupleType( telegramTuple, _TelegramLoadSentResponse );
        it->toValue( telegramTuple[_telegram] );
        m_tupleRouter.route( telegramTuple );
    }

    delete( telegramLoader );
}

void Linda2Responder::send( const Tuple& tuple )
{
    String recipientSnowflake = tuple[_recipientSnowflake];
    TelegramLoader* telegramLoader( m_telegramLoaderFactory.makeLoader( recipientSnowflake ) );

    Telegram telegram( Telegram::fromValue( tuple[_telegram] ) );

    LiteStream stream;
    stream << "Linda2TelegramLoaderResponder: Received TelegramSendRequest from "
           << telegram.m_senderSnowflake;
    LOG_DEBUG( stream.str() );

    bool success( telegramLoader->send( telegram ) );

    LOG_DEBUG( "Linda2TelegramLoaderResponder: Sending TelegramSendResponse" );
    Tuple response;
    TupleRouter::setSourceActor( response, _TelegramLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _TelegramSendResponse );
    response[_success] = success;
    m_tupleRouter.route( response );
}

void Linda2Responder::markRead( const Tuple& tuple )
{
    String recipientSnowflake = tuple[_recipientSnowflake];
    TelegramLoader* telegramLoader( m_telegramLoaderFactory.makeLoader( recipientSnowflake ) );

    Telegram telegram( Telegram::fromValue( tuple[_telegram] ) );

    LiteStream stream;
    stream << "Linda2TelegramLoaderResponder: Received TelegramMarkReadRequest for "
           << telegram.m_telegramSnowflake;
    LOG_DEBUG( stream.str() );

    bool success( telegramLoader->markRead( telegram ) );

    LOG_DEBUG( "Linda2TelegramLoaderResponder: Sending TelegramMakReadResponse" );
    Tuple response;
    TupleRouter::setSourceActor( response, _TelegramLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _TelegramMarkReadResponse );
    response[_success] = success;
    m_tupleRouter.route( response );
}

void Linda2Responder::erase( const Tuple& tuple )
{
    String recipientSnowflake = tuple[_recipientSnowflake];
    TelegramLoader* telegramLoader( m_telegramLoaderFactory.makeLoader( recipientSnowflake ) );

    Telegram telegram( Telegram::fromValue( tuple[_telegram] ) );

    LiteStream stream;
    stream << "Linda2TelegramLoaderResponder: Received TelegramEraseRequest for "
           << telegram.m_telegramSnowflake;
    LOG_DEBUG( stream.str() );

    bool success( telegramLoader->erase( telegram ) );

    LOG_DEBUG( "Linda2TelegramLoaderResponder: Sending TelegramEraseResponse" );
    Tuple response;
    TupleRouter::setSourceActor( response, _TelegramLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _TelegramEraseResponse );
    response[_success] = success;
    m_tupleRouter.route( response );
}

void Linda2Responder::unread( const Tuple& tuple )
{
    String recipientSnowflake = tuple[_recipientSnowflake];
    bool allDevices = ( (int)tuple[_allDevices] == 1 );
    TelegramLoader* telegramLoader( m_telegramLoaderFactory.makeLoader( recipientSnowflake ) );

    LiteStream stream;
    stream << "Linda2TelegramLoaderResponder: Received TelegramUnreadRequest for "
           << recipientSnowflake;
    LOG_DEBUG( stream.str() );

    Map< String, int > numUnread;
    bool success( telegramLoader->unread( numUnread, allDevices) );

    Value numUnreadValue;
    Map< String, int >::const_iterator it( numUnread.begin() );
    for( ; it != numUnread.end(); ++it )
    {
        numUnreadValue[it->first] = it->second;
    }

    LOG_DEBUG( "Linda2TelegramLoaderResponder: Sending TelegramUnreadResponse" );
    Tuple response;
    TupleRouter::setSourceActor( response, _TelegramLoaderResponder );
    TupleRouter::setSourceID( response, m_tupleRouter.myID() );
    TupleRouter::setDestinationID( response, TupleRouter::sourceID( tuple ) );
    TupleRouter::setTupleType( response, _TelegramUnreadResponse );
    response[_success] = success;
    response[_numUnread] = numUnreadValue;
    m_tupleRouter.route( response );
}

} // namespace TelegramLoaders

} // namespace Agape
