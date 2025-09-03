#include "Loggers/Logger.h"
#include "Collections.h"
#include "QueueingTupleRoute.h"
#include "Runnable.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"
#include "TupleRoutingCriteria.h"

#include <condition_variable>
#include <mutex>

namespace Agape
{

namespace Linda2
{

namespace TupleRoutes
{

Queueing::Queueing( const String& routeName ) :
  TupleRoute( routeName ),
  m_partner( nullptr ),
  m_stop( false )
{
}

Queueing::~Queueing()
{
    stop();
}

bool Queueing::haveIncoming()
{
    std::scoped_lock lock( m_mutex );
    return !m_incomingQueue.empty();
}

bool Queueing::receiveTuple( Tuple& tuple )
{
    std::scoped_lock lock( m_mutex );
    if( !m_stop && !m_incomingQueue.empty() )
    {
        //LOG_DEBUG( "QueueingTupleRoute: Receiving" );
        tuple = m_incomingQueue.front();
        m_incomingQueue.pop_front();
        return true;
    }

    return false;
}

void Queueing::run()
{
}

void Queueing::stop()
{
    m_incomingPending.notify_all();
}

bool Queueing::error() const
{
    return false;
}

void Queueing::setPartner( Queueing* partner )
{
    //LOG_DEBUG( "QueueingTupleRoute: Set partner" );
    m_partner = partner;
}

void Queueing::waitIncoming()
{
    std::unique_lock< std::mutex > lock( m_mutex );
    if( m_incomingQueue.empty() )
    {
        //LOG_DEBUG( "QueueingTupleRoute: Waiting for incoming" );
        m_incomingPending.wait( lock );
        //LOG_DEBUG( "QueueingTupleRoute: Incoming pending" );
    }
}

void Queueing::enqueue( const Tuple& tuple )
{
    //LOG_DEBUG( "QueueingTupleRoute: Enqueueing" );
    std::scoped_lock lock( m_mutex );
    m_incomingQueue.push_back( tuple );
    m_incomingPending.notify_all();
}

bool Queueing::_sendTuple( const Tuple& tuple )
{
    // FIXME: Hack to prevent Hydra from routing authentication keys!
    if( TupleRouter::tupleType( tuple ) != _Authenticate )
    {
        if( m_partner != nullptr )
        {
            //LOG_DEBUG( "QueueingTupleRoute: Enqueuing to partner" );
            m_partner->enqueue( tuple );
            return true;
        }
    }

    return false;
}

} // namespace TupleRoutes

} // namespace Linda2

} // namespace Agape
