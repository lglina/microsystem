#include "Loggers/Logger.h"
#include "TupleRoutes/QueueingTupleRoute.h"
#include "Utils/LiteStream.h"
#include "RedisMasterClock.h"
#include "String.h"
#include "StringConstants.h"
#include "StringSerialiser.h"
#include "TupleRouter.h"
#include "Tuple.h"

#include <hiredis/hiredis.h>

#include <chrono>

using namespace std::chrono;
using namespace std::literals;

namespace Agape
{

namespace Stratus
{

MasterClock::MasterClock() :
  m_lastTime( system_clock::now() ),
  m_redisContext( nullptr )
{
}

MasterClock::~MasterClock()
{
    if( m_redisContext )
    {
        redisFree( m_redisContext );
    }
}

void MasterClock::run()
{
    if( !m_redisContext )
    {
        m_redisContext = redisConnect( "127.0.0.1", 6379 );

        if( m_redisContext->err )
        {
            LiteStream stream;
            stream << "RedisMasterClock: Error creating context: "
                << m_redisContext->errstr;
            LOG_DEBUG( stream.str() );
            redisFree( m_redisContext );
            m_redisContext = nullptr;
        }
    }

    if( m_redisContext &&
        ( ( system_clock::now() - m_lastTime ) >= 1s ) )
    {
        m_lastTime += 1s;

        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _Clock );
        TupleRouter::setSourceID( tuple, _Clock );
        TupleRouter::setTupleType( tuple, _Time );
        tuple[_now] = (double)duration_cast< seconds >( system_clock::now().time_since_epoch() ).count();

        StringSerialiser serialiser;
        tuple.toReadableWritable( serialiser );

        redisReply* reply = (redisReply*)redisCommand( m_redisContext,
                                                       "PUBLISH %s %b",
                                                       _Clock,
                                                       serialiser.m_data.data(),
                                                       serialiser.m_data.length() );
        if( !reply )
        {
            LiteStream stream;
            stream << "RedisMasterClock: Error publishing: "
                << m_redisContext->errstr;
            LOG_DEBUG( stream.str() );
        }
    }
}

} // namespace Stratus

} // namespace Agape
