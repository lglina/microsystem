#include "TupleDispatcher.h"

#include "Collections.h"
#include "String.h"

#include "Actors/Actor.h"

#include "TupleRouter.h"

#include "Loggers/Logger.h"

//#include <iostream>

namespace Agape
{

namespace Linda2
{

TupleDispatcher::TupleDispatcher() :
  m_monitor( nullptr )
{
}

void TupleDispatcher::registerActor( Actor* actor )
{
    //std::cerr << "Register " << actor->actorName() << "@" << actor << std::endl;
    m_actors.push_back( actor );
}

void TupleDispatcher::deregisterActor( Actor* actor )
{
    List< Actor* >::iterator it( m_actors.begin() );
    //bool didDeregister( false );
    for( ; it != m_actors.end(); ++it )
    {
        if( *it == actor )
        {
            m_actors.erase( it );
            //std::cerr << "Deregister " << actor->actorName() << "@" << actor << std::endl;
            //didDeregister = true;
            break;
        }
    }

    //if( !didDeregister )
    //{
        //std::cerr << "No registration record for " << actor->actorName() << "@" << actor << "!" << std::endl;
        //exit( 1 );
    //}
}

void TupleDispatcher::registerMonitor( Actor* actor )
{
    m_monitor = actor;
}

void TupleDispatcher::deregisterMonitor( Actor* actor )
{
    m_monitor = nullptr;
}

bool TupleDispatcher::dispatch( Tuple& tuple )
{
    if( m_monitor )
    {
        m_monitor->accept( tuple );
    }

    bool handled( false );
    String destinationActor( TupleRouter::destinationActor( tuple ) );
    for( List< Actor* >::iterator it( m_actors.begin() ); it != m_actors.end(); ++it )
    {
        if( destinationActor.empty() || ( destinationActor == ( *it )->actorName() ) )
        {
            if( ( *it )->accept( tuple ) )
            {
#ifdef LOG_TUPLES
                LOG_DEBUG( "TupleDispatcher: " + ( *it )->actorName() + " accepted tuple" );
#endif
                handled = true;
                // N.B. Using a list here as actors may mutate the actor list
                // as a result of handling a tuple (e.g. by creating and
                // deleting other actors), and this could invalidate a Vector
                // iterator. It is assumed that an actor will not erase itself.
                // This also implies that it is unsafe for an actor to do
                // anything that would cause it to be externally reloaded
                // (destroyed and recreated).
            }
        }
    }

    return handled;
}

} // namespace Linda2

} // namespace Agape
