#include "Loggers/Logger.h"
#include "ReadableWritableTupleRoute.h"
#include "ReadableWritable.h"
#include "String.h"

namespace Agape
{

namespace Linda2
{

namespace TupleRoutes
{

ReadableWritable::ReadableWritable( const String& routeName, Agape::ReadableWritable& rw ) :
  TupleRoute( routeName ),
  m_rw( rw )
{
}

bool ReadableWritable::haveIncoming()
{
    return true;
}

bool ReadableWritable::receiveTuple( Tuple& tuple )
{
    //LOG_DEBUG( "ReadableWritableTupleRoute: Receiving" );
    bool haveTuple( Tuple::fromReadableWritable( m_rw, tuple ) );
    return haveTuple;
}

void ReadableWritable::run()
{
}

bool ReadableWritable::error() const
{
    return m_rw.error();
}

bool ReadableWritable::_sendTuple( const Tuple& tuple )
{
    //LOG_DEBUG( "ReadableWritableTupleRoute: Sending" );
    if( !m_rw.error() )
    {
        bool retval( tuple.toReadableWritable( m_rw ) );
        if( retval ) m_rw.flushOutput(); // If buffering, write/send now.
        return retval;
    }

    return false;
}

}

} // namespace Linda2

} // namespace Agape
