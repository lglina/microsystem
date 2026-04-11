#include "Loggers/Logger.h"
#include "World/WorldCoordinates.h"
#include "Authenticator.h"
#include "StratusTupleFilter.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"

using namespace Agape::Stratus;

namespace Agape
{

namespace Linda2
{

namespace TupleFilters
{

Stratus::Stratus( Authenticator& authenticator ) :
  m_authenticator( authenticator )
{
}

bool Stratus::permitIn( const Tuple& tuple )
{
    bool permittedIn( false );

    if( m_authenticator.credentialsValid() )
    {
        if( TupleRouter::tupleType( tuple ) == _RoutingCriteria )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleFilters::Stratus: Routing criteria permitted" );
#endif
            permittedIn = true;
        }
        else if( ( TupleRouter::sourceActor( tuple ) == _TelegramLoader ) ||
                 ( TupleRouter::sourceActor( tuple ) == _WorldLoader ) )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleFilters::Stratus: Global loader requests permitted" );
#endif
            permittedIn = true;
        }
        else if( ( TupleRouter::sourceActor( tuple ) == _AssetLoader ) &&
                 ( World::Coordinates::fromValue( tuple[_coordinates] ).m_worldID == _sharedAssetsWorldID ) )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleFilters::Stratus: Shared asset store requests permitted" );
#endif
            permittedIn = true;
        }
        else if( TupleRouter::sourceActor( tuple ) == _InviteFriendClient )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleFilters::Stratus: Invitation requests permitted" );
#endif
            permittedIn = true;
        }
        else if( TupleRouter::sourceActor( tuple ) == _UpdateClient )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleFilters::Stratus: Update requests permitted" );
#endif
            permittedIn = true;
        }
        else
        {
            World::Coordinates coordinates( World::Coordinates::fromValue( tuple[_coordinates] ) );
            permittedIn = m_authenticator.joinedWorld( coordinates.m_worldID );

            if( permittedIn )
            {
#ifdef LOG_TUPLES
                LOG_DEBUG( "TupleFilters::Stratus: Permitted for world ID " + coordinates.m_worldID );
#endif
            }
            else if( !coordinates.m_worldID.empty() )
            {
#ifdef LOG_TUPLES
                LOG_DEBUG( "TupleFilters::Stratus: Forbidden for world ID " + coordinates.m_worldID );
#endif
            }
            else
            {
#ifdef LOG_TUPLES
                LOG_DEBUG( "TupleFilters::Stratus: World ID not found in tuple. Forbidden." );
#endif
            }
        }
    }
    else if( TupleRouter::tupleType( tuple ) == _Authenticate )
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "TupleFilters::Stratus: Authentication permitted." );
#endif
        permittedIn = true;
    }
    else
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "TupleFilters::Stratus: Tuple forbidden - no valid authentication." );
#endif
    }

    return permittedIn;
}

bool Stratus::permitInDefault( const Tuple& tuple ) // In *from* default route.
{
    bool permittedInDefault( false );

    if( ( TupleRouter::tupleType( tuple ) == _Time ) && !tuple.hasValue( _coordinates ) )
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "TupleFilters::Stratus: Time tuples permitted" );
#endif
        permittedInDefault = true;
    }
    else
    {
        World::Coordinates coordinates( World::Coordinates::fromValue( tuple[_coordinates] ) );
        permittedInDefault = m_authenticator.joinedWorld( coordinates.m_worldID );

        if( permittedInDefault )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleFilters::Stratus: Permitted for world ID " + coordinates.m_worldID );
#endif
        }
        else if( !coordinates.m_worldID.empty() )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleFilters::Stratus: Forbidden for world ID " + coordinates.m_worldID );
#endif
        }
        else
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleFilters::Stratus: World ID not found in tuple. Forbidden." );
#endif
        }
    }

    return permittedInDefault;
}

bool Stratus::permitForward( const Tuple& tuple )
{
    bool permittedForward( false );

    if( TupleRouter::tupleType( tuple ) == _Authenticate ) // Don't allow leaking of secrets!
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "TupleFilters::Stratus: Not forwarding authentication request." );
#endif
    }
    else
    {
        World::Coordinates coordinates( World::Coordinates::fromValue( tuple[_coordinates] ) );
        permittedForward = m_authenticator.writableWorld( coordinates.m_worldID );

        if( permittedForward )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleFilters::Stratus: Forwarding permitted for world ID " + coordinates.m_worldID );
#endif
        }
        else if( !coordinates.m_worldID.empty() )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleFilters::Stratus: Forwarding forbidden for world ID " + coordinates.m_worldID );
#endif
        }
        else
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleFilters::Stratus: World ID not found in tuple. Forwarding forbidden." );
#endif
        }
    }

    return permittedForward;
}

bool Stratus::permitForwardDefault( const Tuple& tuple ) // Forward *from* default route.
{
    bool permittedForwardDefault( false );

    if( ( TupleRouter::tupleType( tuple ) == _Time ) && !tuple.hasValue( _coordinates ) )
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "TupleFilters::Stratus: Forwarding permitted for time tuples from default route" );
#endif
        permittedForwardDefault = true;
    }
    else
    {
        permittedForwardDefault = m_authenticator.credentialsValid();

        if( permittedForwardDefault )
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleFilters::Stratus: Permitting forwarding of allowed tuple from default route" );
#endif
        }
        else
        {
#ifdef LOG_TUPLES
            LOG_DEBUG( "TupleFilters::Stratus: Forwarding from default route forbidden - no valid authentication." );
#endif
        }
    }

    return permittedForwardDefault;
}

bool Stratus::permitOut( const Tuple& tuple )
{
#ifdef LOG_TUPLES
    LOG_DEBUG( "TupleFilters::Stratus: Permitting sending to endpoint unconditionally" );
#endif
    return true;
}

bool Stratus::permitOutDefault( const Tuple& tuple ) // Out *to* default route.
{
    bool permittedOutDefault( false );

    // Permit all with coordinates for writable worlds.
    World::Coordinates coordinates( World::Coordinates::fromValue( tuple[_coordinates] ) );
    permittedOutDefault = m_authenticator.writableWorld( coordinates.m_worldID );

    // Don't send out responses from our own handler responders, unless we want
    // those responses to propagate to other clients in the same world. Note
    // that other handlers will decline to send tuples to their clients that
    // they don't have routing criteria for, but we want to prevent guff getting
    // out to the world-global message bus as much as possible.
    String sourceActor( TupleRouter::sourceActor( tuple ) );
    if( ( sourceActor == _AssetLoaderResponder ) ||
        ( sourceActor == _PresenceLoaderResponder ) ||
        ( sourceActor == _SceneLoaderResponder ) )
    {
        String tupleType( TupleRouter::tupleType( tuple ) );
        if( ( tupleType != _PresenceResponse ) &&
            ( tupleType != _SceneResponse ) &&
            ( tupleType != _SceneItemSaveAttributeResponse ) &&
            ( tupleType != _SceneItemDeleteAttributesResponse ) &&
            ( tupleType != _InvalidateCachedAsset ) )
        {
            permittedOutDefault = false;
        }
    }

    if( permittedOutDefault )
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "TupleFilters::Stratus: Permitting sending joined world tuples to default route" );
#endif
    }
    else
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "TupleFilters::Stratus: Sending to default route forbidden - tuple for non-writable world, tuple has no coordinates or tuple is a local response" );
#endif
    }

    return permittedOutDefault;
}

} // namespace TupleFilters

} // namespace Linda2

} // namespace Agape
