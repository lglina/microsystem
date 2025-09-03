#include "NullEventClock.h"
#include "TupleRouter.h"

namespace Agape
{

namespace EventClocks
{

Null::Null( TupleRouter& tupleRouter ) :
  EventClock( tupleRouter )
{
}
    
void Null::run()
{
}

} // namespace EventClocks

} // namespace Agape
