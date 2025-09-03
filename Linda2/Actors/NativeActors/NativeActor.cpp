#include "NativeActor.h"

#include "String.h"

namespace Agape
{

namespace Linda2
{

namespace Actors
{

Native::Native( const String& name ) :
  m_actorName( name )
{
}

String Native::actorName() const
{
    return m_actorName;
}

void Native::str( LiteStream& stream, int index )
{
}

} // namespace Actors

} // namespace Linda2

} // namespace Agape