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
                LOG_DEBUG( "TupleFilters::Stratus: Forbidden for world ID " + coordinates.m_worldID );
            }
            else
            {
                LOG_DEBUG( "TupleFilters::Stratus: World ID not found in tuple. Forbidden." );
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
        LOG_DEBUG( "TupleFilters::Stratus: Tuple forbidden - no valid authentication." );
    }

    return permittedIn;
}

bool Stratus::permitInDefault( const Tuple& tuple )
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
            LOG_DEBUG( "TupleFilters::Stratus: Forbidden for world ID " + coordinates.m_worldID );
        }
        else
        {
            LOG_DEBUG( "TupleFilters::Stratus: World ID not found in tuple. Forbidden." );
        }
    }

    return permittedInDefault;
}

bool Stratus::permitForward( const Tuple& tuple )
{
    bool permittedForward( false );

    if( TupleRouter::tupleType( tuple ) == _Authenticate ) // Don't allow leaking of secrets!
    {
        LOG_DEBUG( "TupleFilters::Stratus: Not forwarding authentication request." );
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
            LOG_DEBUG( "TupleFilters::Stratus: Forwarding forbidden for world ID " + coordinates.m_worldID );
        }
        else
        {
            LOG_DEBUG( "TupleFilters::Stratus: World ID not found in tuple. Forwarding forbidden." );
        }
    }

    return permittedForward;
}

bool Stratus::permitForwardDefault( const Tuple& tuple )
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
            LOG_DEBUG( "TupleFilters::Stratus: Forwarding from default route forbidden - no valid authentication." );
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

bool Stratus::permitOutDefault( const Tuple& tuple )
{
    bool permittedOutDefault( false );

    World::Coordinates coordinates( World::Coordinates::fromValue( tuple[_coordinates] ) );
    permittedOutDefault = m_authenticator.writableWorld( coordinates.m_worldID );

    if( permittedOutDefault )
    {
#ifdef LOG_TUPLES
        LOG_DEBUG( "TupleFilters::Stratus: Permitting sending joined world tuples to default route" );
#endif
    }
    else
    {
        LOG_DEBUG( "TupleFilters::Stratus: Sending to default route forbidden - tuple for non-writable world or tuple has no coordinates" );
    }

    return permittedOutDefault;
}

} // namespace TupleFilters

} // namespace Linda2

} // namespace Agape
