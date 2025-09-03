#include "Clocks/Clock.h"
#include "World/ScenePresence.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "PresenceRequest.h"
#include "OfflinePresenceLoader.h"
#include "OfflinePresenceStore.h"
#include "String.h"

using namespace Agape::World;

namespace Agape
{

namespace PresenceLoaders
{
       
Offline::Offline( const World::Coordinates& coordinates,
                  OfflinePresenceStore& offlinePresenceStore,
                  Clock& clock ) :
  PresenceLoader( coordinates ),
  m_offlinePresenceStore( offlinePresenceStore ),
  m_clock( clock )
{
}

bool Offline::load( Vector< World::ScenePresence >& scenePresences )
{
    Map< String, ScenePresence >::const_iterator it( m_offlinePresenceStore.m_presences.begin() );
    for( ; it != m_offlinePresenceStore.m_presences.end(); ++it )
    {
        if( it->second.m_coordinates == m_coordinates )
        {
            scenePresences.push_back( it->second );
        }
    }

    return true;
}

bool Offline::loadWorld( Vector< ScenePresence >& worldPresences )
{
    Vector< ScenePresence > matchingPresences;
    Map< String, ScenePresence >::const_iterator it( m_offlinePresenceStore.m_presenceHistory.begin() );
    for( ; it != m_offlinePresenceStore.m_presenceHistory.end(); ++it )
    {
        if( it->second.m_coordinates.m_worldID == m_coordinates.m_worldID )
        {
            worldPresences.push_back( it->second );
        }
    }

    return true;
}

bool Offline::request( const Vector< PresenceRequest >& requests )
{
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

    // FIXME: Return success/error?
    return true;
}

Vector< PresenceRequest > Offline::getUpdates()
{
    return Vector< PresenceRequest >();
}

void Offline::_arrive( const PresenceRequest& request )
{
    // Add to presences and presence history
    ScenePresence presence( request.m_scenePresence );
    presence.m_lastSeen = m_clock.epochS();
    String snowflake( presence.m_user.m_snowflake );
    m_offlinePresenceStore.m_presences[snowflake] = presence;
    m_offlinePresenceStore.m_presenceHistory[snowflake] = presence;
}

void Offline::_depart( const PresenceRequest& request )
{
    String snowflake( request.m_scenePresence.m_user.m_snowflake );

    // Delete from presences
    {
    std::map< String, ScenePresence >::const_iterator it( m_offlinePresenceStore.m_presences.find( snowflake ) );
    if( it != m_offlinePresenceStore.m_presences.end() )
    {
        m_offlinePresenceStore.m_presences.erase( it );
    }
    }

    // Set "present" in history to false
    {
    std::map< String, ScenePresence >::const_iterator it( m_offlinePresenceStore.m_presenceHistory.find( snowflake ) );
    if( it != m_offlinePresenceStore.m_presenceHistory.end() )
    {
        m_offlinePresenceStore.m_presenceHistory[snowflake].m_present = false;
    }
    }
}

void Offline::_move( const PresenceRequest& request )
{
    ScenePresence presence( request.m_scenePresence );
    presence.m_lastSeen = m_clock.epochS();
    String snowflake( presence.m_user.m_snowflake );

    // Overwrite presence in presences and history
    {
    std::map< String, ScenePresence >::const_iterator it( m_offlinePresenceStore.m_presences.find( snowflake ) );
    if( it != m_offlinePresenceStore.m_presences.end() )
    {
        m_offlinePresenceStore.m_presences[snowflake] = presence;
    }
    }

    {
    std::map< String, ScenePresence >::const_iterator it( m_offlinePresenceStore.m_presenceHistory.find( snowflake ) );
    if( it != m_offlinePresenceStore.m_presenceHistory.end() )
    {
        m_offlinePresenceStore.m_presenceHistory[snowflake] = presence;
    }
    }
}

} // namespace PresenceLoaders

} // namespace Agape
