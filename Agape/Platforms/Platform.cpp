#include "Collections.h"
#include "Platform.h"

namespace Agape
{

void Platform::registerListener( EventListener* listener )
{
    m_eventListeners.push_back( listener );
}

void Platform::dispatchEvent( const Event& event )
{
    for( Vector< EventListener* >::const_iterator it( m_eventListeners.begin() ); it != m_eventListeners.end(); ++it )
    {
        ( *it )->receiveEvent( event );
    }
}

} // namespace Agape
