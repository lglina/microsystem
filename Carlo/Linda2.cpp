#include "Actors/Linda2Actor.h"
#include "Utils/LiteStream.h"
#include "Collections.h"
#include "ExecutionContext.h"
#include "Linda2.h"
#include "Value.h"

namespace Agape
{

namespace Carlo
{

Linda2::~Linda2()
{
    Vector< Agape::Linda2::Actors::Linda2Actor* >::iterator it( m_actors.begin() );
    for( ; it != m_actors.end(); ++it )
    {
        delete( *it );
    }
}

void Linda2::str( LiteStream& stream )
{
    stream << "Linda2\n{\n";
    Vector< Agape::Linda2::Actors::Linda2Actor* >::iterator it( m_actors.begin() );
    for( ; it != m_actors.end(); ++it )
    {
        ( *it )->str( stream, 4 );
    }
    stream << "}\n";
}

bool Linda2::evalOne( Value& value, ExecutionContext& executionContext )
{
    if( !m_actors.empty() )
    {
        return ( *( m_actors.begin() ) )->evalOne( value, executionContext );
    }

    return false;
}

} // namespace Linda2

} // namespace Agape
