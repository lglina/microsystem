#include "Actors/NativeActors/NativeActor.h"
#include "Loggers/Logger.h"
#include "Network/WebSocketsConnection.h"
#include "PresenceLoaders/SharedPresenceStore.h"
#include "TelegramLoaders/Factories/TelegramLoadersFactory.h"
#include "TelegramLoaders/TelegramLoader.h"
#include "Utils/LiteStream.h"
#include "World/ScenePresence.h"
#include "Authenticator.h"
#include "Collections.h"
#include "PushNotifier.h"
#include "String.h"
#include "StringConstants.h"
#include "Tuple.h"
#include "TupleRouter.h"

#include <chrono>
#include <mutex>

namespace
{
    const int checkTelegramsPeriod( 300 ); // s.
    const int idleTime( 300 ); // s.
} // Anonymous namespace

namespace Agape
{

using namespace Linda2;
using namespace Network;
using namespace PresenceLoaders;
using namespace World;

namespace Stratus
{

PushNotifier::PushNotifier( TupleRouter& tupleRouter,
                            const Authenticator& authenticator,
                            const SharedPresenceStore& sharedPresenceStore,
                            TelegramLoaders::Factory& telegramLoaderFactory,
                            WebSocketsConnection& webSocketsConnection ) :
  m_tupleRouter( tupleRouter ),
  m_authenticator( authenticator ),
  m_sharedPresenceStore( sharedPresenceStore ),
  m_telegramLoaderFactory( telegramLoaderFactory ),
  m_webSocketsConnection( webSocketsConnection ),
  m_lastCheckedTelegrams( 0 ),
  m_lastUnreadTelegrams( -1 ),
  Actors::Native( _PushNotifier )
{
    m_tupleRouter.registerActor( this );
}

PushNotifier::~PushNotifier()
{
    m_tupleRouter.deregisterActor( this );
}

bool PushNotifier::accept( Tuple& tuple )
{
    bool handled( false );

    String tupleType( TupleRouter::tupleType( tuple ) );
    if( tupleType == _ChatMessage )
    {
        LiteStream stream;
        stream << "PushNotifier: Received chat message from user with snowflake "
               << tuple[_user][_snowflake].toString()
               << ". Our user? "
               << ( m_authenticator.isOurUser( tuple[_user][_snowflake] ) ? 1 : 0 )
               << ". Idle? "
               << (  isIdle() ? 1 : 0 );
        LOG_DEBUG( stream.str() );
        if( !m_authenticator.isOurUser( tuple[_user][_snowflake] ) &&
            isIdle() )
        {
            LOG_DEBUG( "PushNotifier: Sending chat notification" );
            m_webSocketsConnection.sendOutOfBand( "notification.chat" );
        }
    }
    else if( ( tupleType == _Time ) && m_authenticator.credentialsValid() )
    {
        long long timeNow( (long long)( (double)tuple[_now]) );
        if( ( timeNow - m_lastCheckedTelegrams ) >= checkTelegramsPeriod )
        {
            LOG_DEBUG( "PushNotifier: Checking telegrams" );
            TelegramLoader* telegramLoader( m_telegramLoaderFactory.makeLoader( String() ) );
            Map< String, int > numUnread;
            // Find unread for all worlds for all devices if Tela, as Tela
            // devices have access to all joined worlds from all devices.
            if( telegramLoader->unread( numUnread, m_authenticator.isTela() ) )
            {
                int totalUnread( 0 );
                Map< String, int >::const_iterator it( numUnread.begin() );
                for( ; it != numUnread.end(); ++it )
                {
                    totalUnread += it->second;
                }

                LiteStream stream;
                stream << "PushNotifier: Last unread telegrams "
                       << m_lastUnreadTelegrams
                       << ". Unread now "
                       << totalUnread
                       << ". Idle? "
                       << (  isIdle() ? 1 : 0 );
                LOG_DEBUG( stream.str() );

                if( ( m_lastUnreadTelegrams != -1 ) &&
                    ( totalUnread != m_lastUnreadTelegrams ) &&
                    isIdle() )
                {
                    LOG_DEBUG( "PushNotifier: Sending telegram notification" );
                    m_webSocketsConnection.sendOutOfBand( "notification.telegram" );
                }

                m_lastUnreadTelegrams = totalUnread;
                m_lastCheckedTelegrams = timeNow;
            }
            delete( telegramLoader );
        }
    }

    return handled;
}

bool PushNotifier::isIdle()
{
    std::scoped_lock lock( m_sharedPresenceStore.m_mutex );

    // If something went wrong with the user departing a world we might have
    // them as active in the current world but idle in the old one, so the
    // user is *not* idle if we find them non-idle in *any* world.
    bool foundNonIdle( false );

    Map< String, ScenePresence >::const_iterator it( m_sharedPresenceStore.m_presences.begin() );
    for( ; it != m_sharedPresenceStore.m_presences.end(); ++it )
    {
        const String& snowflake( it->first );
        const ScenePresence& scenePresence( it->second );
        auto currentTime = std::chrono::system_clock::now();
        long long sinceEpoch = std::chrono::duration_cast< std::chrono::seconds >( currentTime.time_since_epoch() ).count();
        if( m_authenticator.isOurUser( it->first ) &&
            ( ( sinceEpoch - scenePresence.m_lastSeen ) < idleTime ) )
        {
            foundNonIdle = true;
        }
    }

    return !foundNonIdle;
}

} // namespace Stratus

} // namespace Agape
