#include "Clocks/Clock.h"
#include "World/ScenePresence.h"
#include "World/WorldCoordinates.h"
#include "Authenticator.h"
#include "Collections.h"
#include "PresenceRequest.h"
#include "SharedPresenceLoader.h"
#include "SharedPresenceStore.h"
#include "String.h"

#include <mutex>

using namespace Agape::Stratus;
using namespace Agape::World;

namespace Agape
{

namespace PresenceLoaders
{
       
Shared::Shared( const World::Coordinates& coordinates,
                SharedPresenceStore& sharedPresenceStore,
                Clock& clock,
                Authenticator& authenticator ) :
  PresenceLoader( coordinates ),
  m_sharedPresenceStore( sharedPresenceStore ),
  m_clock( clock ),
  m_authenticator( authenticator )
{
}

bool Shared::load( Vector< World::ScenePresence >& scenePresences )
{
    std::scoped_lock lock( m_sharedPresenceStore.m_mutex );

    Map< String, ScenePresence >::const_iterator it( m_sharedPresenceStore.m_presences.begin() );
    for( ; it != m_sharedPresenceStore.m_presences.end(); ++it )
    {
        if( it->second.m_coordinates == m_coordinates )
        {
            scenePresences.push_back( it->second );
        }
    }

    return true;
}

bool Shared::loadWorld( Vector< ScenePresence >& worldPresences )
{
    std::scoped_lock lock( m_sharedPresenceStore.m_mutex );

    Vector< ScenePresence > matchingPresences;
    Map< String, ScenePresence >::const_iterator it( m_sharedPresenceStore.m_presenceHistory.begin() );
    for( ; it != m_sharedPresenceStore.m_presenceHistory.end(); ++it )
    {
        if( it->second.m_coordinates.m_worldID == m_coordinates.m_worldID )
        {
            worldPresences.push_back( it->second );
        }
    }

    return true;
}

bool Shared::request( const Vector< PresenceRequest >& requests )
{
    if( m_authenticator.writableWorld( m_coordinates.m_worldID ) )
    {
        std::scoped_lock lock( m_sharedPresenceStore.m_mutex );

        Vector< PresenceRequest >::const_iterator it( requests.begin() );
        for( ; it != requests.end(); ++it )
        {
            switch( it->m_presenceOperation )
            {
            case PresenceRequest::arrive:
                _arrive( *it );
                break;
            case PresenceRequest::depart:
                _depart( *it );
                break;
            case PresenceRequest::move:
                _move( *it );
                break;
            default:
                break;
            }
        }
    }

    // FIXME: Return success/error?
    return true;
}

Vector< PresenceRequest > Shared::getUpdates()
{
    return Vector< PresenceRequest >();
}

void Shared::_arrive( const PresenceRequest& request )
{
    // Pre-requisite: m_sharedPresenceStore.m_mutex locked.
    // Add to presences and presence history
    ScenePresence presence( request.m_scenePresence );
    presence.m_lastSeen = m_clock.epochS();
    String snowflake( presence.m_user.m_snowflake );
    m_sharedPresenceStore.m_presences[snowflake] = presence;
    m_sharedPresenceStore.m_presenceHistory[snowflake] = presence;
}

void Shared::_depart( const PresenceRequest& request )
{
    // Pre-requisite: m_sharedPresenceStore.m_mutex locked.
    String snowflake( request.m_scenePresence.m_user.m_snowflake );

    // Delete from presences
    {
    std::map< String, ScenePresence >::const_iterator it( m_sharedPresenceStore.m_presences.find( snowflake ) );
    if( it != m_sharedPresenceStore.m_presences.end() )
    {
        m_sharedPresenceStore.m_presences.erase( it );
    }
    }

    // Set "present" in history to false
    {
    std::map< String, ScenePresence >::const_iterator it( m_sharedPresenceStore.m_presenceHistory.find( snowflake ) );
    if( it != m_sharedPresenceStore.m_presenceHistory.end() )
    {
        m_sharedPresenceStore.m_presenceHistory[snowflake].m_present = false;
    }
    }
}

void Shared::_move( const PresenceRequest& request )
{
    // Pre-requisite: m_sharedPresenceStore.m_mutex locked.
    ScenePresence presence( request.m_scenePresence );
    presence.m_lastSeen = m_clock.epochS();
    String snowflake( presence.m_user.m_snowflake );

    // Overwrite presence in presences and history
    {
    std::map< String, ScenePresence >::const_iterator it( m_sharedPresenceStore.m_presences.find( snowflake ) );
    if( it != m_sharedPresenceStore.m_presences.end() )
    {
        m_sharedPresenceStore.m_presences[snowflake] = presence;
    }
    }

    {
    std::map< String, ScenePresence >::const_iterator it( m_sharedPresenceStore.m_presenceHistory.find( snowflake ) );
    if( it != m_sharedPresenceStore.m_presenceHistory.end() )
    {
        m_sharedPresenceStore.m_presenceHistory[snowflake] = presence;
    }
    }
}

} // namespace PresenceLoaders

} // namespace Agape
