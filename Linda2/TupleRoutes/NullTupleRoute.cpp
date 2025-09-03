#include "NullTupleRoute.h"
#include "String.h"

namespace Agape
{

namespace Linda2
{

namespace TupleRoutes
{

Null::Null( const String& routeName ) :
  TupleRoute( routeName )
{
}

bool Null::haveIncoming()
{
    return false;
}

bool Null::receiveTuple( Tuple& tuple )
{
    return false;
}

void Null::sendTuple( const Tuple& tuple )
{
}

void Null::run()
{
}

bool Null::error() const
{
    return false;
}

bool Null::_sendTuple( const Tuple& tuple )
{
    return true;
}

}

} // namespace Linda2

} // namespace Agape
