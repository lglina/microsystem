#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/Linda2AssetLoaderResponder.h"
#include "Clocks/Clock.h"
#include "Encryptors/Factories/EncryptorsFactory.h"
#include "Encryptors/Encryptor.h"
#include "Encryptors/Hash.h"
#include "EntropySources/EntropySource.h"
#include "Loggers/Logger.h"
#include "Network/WebSocketsConnection.h"
#include "PresenceLoaders/Factories/PresenceLoadersFactory.h"
#include "PresenceLoaders/Linda2PresenceLoaderResponder.h"
#include "SceneLoaders/Factories/SceneLoadersFactory.h"
#include "SceneLoaders/Linda2SceneLoaderResponder.h"
#include "TelegramLoaders/Factories/TelegramLoadersFactory.h"
#include "TelegramLoaders/Linda2TelegramLoaderResponder.h"
#include "TelegramLoaders/Linda2TelegramLoaderResponder.h"
#include "Timers/Factories/TimerFactory.h"
#include "TupleFilters/TupleFilter.h"
#include "TupleRoutes/QueueingTupleRoute.h"
#include "TupleRoutes/TupleRoute.h"
#include "WorldLoaders/Factories/WorldLoadersFactory.h"
#include "WorldLoaders/Linda2WorldLoaderResponder.h"
#include "Authenticator.h"
#include "Handler.h"
#include "Hydra.h"
#include "Inviter.h"
#include "KeyUtilities.h"
#include "PushNotifier.h"
#include "RWBuffer.h"
#include "TupleDispatcher.h"
#include "TupleRouter.h"

#include <functional>
#include <memory>
#include <thread>

#include <unistd.h>

namespace Agape
{

namespace Stratus
{

Handler::Handler( EntropySource* entropySource,
                  Encryptors::Factory* encryptorFactory,
                  Encryptor* encryptor,
                  Hash* hash,
                  KeyUtilities* keyUtilities,
                  Authenticator* authenticator,
                  AssetLoaders::Factory* assetLoaderFactory,
                  AssetLoaders::Linda2Responder* assetLoaderResponder,
                  AssetLoaders::Factory* programAssetLoaderFactory,
                  AssetLoaders::Linda2Responder* programAssetLoaderResponder,
                  AssetLoaders::Factory* telegramAssetLoaderFactory,
                  AssetLoaders::Linda2Responder* telegramAssetLoaderResponder,
                  TelegramLoaders::Factory* telegramLoaderFactory,
                  TelegramLoaders::Linda2Responder* telegramLoaderResponder,
                  PresenceLoaders::Factory* presenceLoaderFactory,
                  PresenceLoaders::Linda2Responder* presenceLoaderResponder,
                  SceneLoaders::Factory* sceneLoaderFactory,
                  SceneLoaders::Linda2Responder* sceneLoaderResponder,
                  WorldLoaders::Factory* worldLoaderFactory,
                  WorldLoaders::Linda2Responder* worldLoaderResponder,
                  Linda2::TupleDispatcher* tupleDispatcher,
                  Linda2::TupleFilter* tupleFilter,
                  Linda2::TupleRouter* tupleRouter,
                  Network::WebSocketsConnection* webSocketsConnection,
                  RWBuffer* rwBuffer,
                  Linda2::TupleRoute* incomingTupleRoute,
                  Hydra& hydra,
                  Linda2::TupleRoutes::Queueing* hydraNearTupleRoute,
                  Linda2::TupleRoutes::Queueing* hydraFarTupleRoute,
                  Timers::Factory* timerFactory,
                  Agape::Clock* clock,
                  Inviter* inviter,
                  PushNotifier* pushNotifier ) :
  m_entropySource( entropySource ),
  m_encryptorFactory( encryptorFactory ),
  m_encryptor( encryptor ),
  m_hash( hash ),
  m_keyUtilities( keyUtilities ),
  m_authenticator( authenticator ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_assetLoaderResponder( assetLoaderResponder ),
  m_programAssetLoaderFactory( programAssetLoaderFactory ),
  m_programAssetLoaderResponder( programAssetLoaderResponder ),
  m_telegramAssetLoaderFactory( telegramAssetLoaderFactory ),
  m_telegramAssetLoaderResponder( telegramAssetLoaderResponder ),
  m_telegramLoaderFactory( telegramLoaderFactory ),
  m_telegramLoaderResponder( telegramLoaderResponder ),
  m_presenceLoaderFactory( presenceLoaderFactory ),
  m_presenceLoaderResponder( presenceLoaderResponder ),
  m_sceneLoaderFactory( sceneLoaderFactory ),
  m_sceneLoaderResponder( sceneLoaderResponder ),
  m_worldLoaderFactory( worldLoaderFactory ),
  m_worldLoaderResponder( worldLoaderResponder ),
  m_tupleDispatcher( tupleDispatcher ),
  m_tupleFilter( tupleFilter ),
  m_tupleRouter( tupleRouter ),
  m_webSocketsConnection( webSocketsConnection ),
  m_rwBuffer( rwBuffer ),
  m_incomingTupleRoute( incomingTupleRoute ),
  m_hydra( hydra ),
  m_hydraNearTupleRoute( hydraNearTupleRoute ),
  m_hydraFarTupleRoute( hydraFarTupleRoute ),
  m_timerFactory( timerFactory ),
  m_clock( clock ),
  m_inviter( inviter ),
  m_pushNotifier( pushNotifier ),
  m_stopping( false ),
  m_stopped( false )
{
}

Handler::~Handler()
{
    m_stopping = true;

    m_hydraNearTupleRoute->stop(); // Stops outgoing thread wait.
    m_outgoingTuplesThread->join();

    m_webSocketsConnection->stop(); // Stops incoming thread wait.
    m_incomingTuplesThread->join();

    m_presenceLoaderResponder->forceDepart();
    m_hydra.signalIncoming(); // Ask Hydra to handle depart requests now.
    usleep(1); // Yield to other threads to do routing.

    m_hydra.removeRoute( m_hydraFarTupleRoute );

    delete( m_webSocketsConnection );

    delete( m_assetLoaderFactory );
    delete( m_assetLoaderResponder );
    delete( m_programAssetLoaderFactory );
    delete( m_programAssetLoaderResponder );
    delete( m_telegramAssetLoaderFactory );
    delete( m_telegramAssetLoaderResponder );
    delete( m_telegramLoaderFactory );
    delete( m_telegramLoaderResponder );
    delete( m_presenceLoaderFactory );
    delete( m_presenceLoaderResponder );
    delete( m_sceneLoaderFactory );
    delete( m_sceneLoaderResponder );
    delete( m_worldLoaderFactory );
    delete( m_worldLoaderResponder );
    delete( m_authenticator );
    delete( m_keyUtilities );
    delete( m_inviter );
    delete( m_pushNotifier );
    delete( m_tupleDispatcher );
    delete( m_tupleFilter );
    delete( m_tupleRouter );
    delete( m_rwBuffer );
    delete( m_incomingTupleRoute );
    delete( m_timerFactory );
    delete( m_clock );

    delete( m_hydraNearTupleRoute );
    delete( m_hydraFarTupleRoute );

    delete( m_entropySource );
    delete( m_encryptor );
    delete( m_hash );
}

void Handler::handle()
{
    m_incomingTuplesThread.reset( new std::thread( std::bind( &Handler::_handleIncoming, this ) ) );
    m_outgoingTuplesThread.reset( new std::thread( std::bind( &Handler::_handleOutgoing, this ) ) );
}

bool Handler::stopped()
{
    return m_stopped;
}

void Handler::_handleIncoming()
{
    LOG_DEBUG( "Handler: Incoming thread starting" );

    while( !m_stopping )
    {
#ifdef LOG_STRATUS
        LOG_DEBUG( "Handler: Waiting for incoming data" );
#endif
        m_webSocketsConnection->waitIncoming();

        if( !m_stopping )
        {
#ifdef LOG_STRATUS
            LOG_DEBUG( "Handler: Running TupleRouter to handle incoming" );
#endif
            std::scoped_lock lock( m_mutex );
            m_tupleRouter->run();

#ifdef LOG_STRATUS
            LOG_DEBUG( "Handler: Signalling Hydra" );
#endif
            m_hydra.signalIncoming();

            if( m_tupleRouter->routeError() )
            {
                LOG_DEBUG( "Handler: Incoming: Router error. Stopping thread." );
                m_stopping = true;
            }
        }
    }

    m_stopped = true;
    LOG_DEBUG( "Handler: Incoming thread stopped" );
}

void Handler::_handleOutgoing()
{
    LOG_DEBUG( "Handler: Outgoing thread starting" );
    while( !m_stopping )
    {
#ifdef LOG_STRATUS
        LOG_DEBUG( "Handler: Waiting for Hydra" );
#endif
        m_hydraNearTupleRoute->waitIncoming();

        if( !m_stopping )
        {
#ifdef LOG_STRATUS
            LOG_DEBUG( "Handler: Running TupleRouter to handle outgoing" );
#endif
            std::scoped_lock lock( m_mutex );
            m_tupleRouter->run();

            if( m_tupleRouter->routeError() )
            {
                LOG_DEBUG( "Handler: Outgoing: Router error. Stopping thread." );
                m_stopping = true;
            }
        }
    }

    m_stopped = true;
    LOG_DEBUG( "Handler: Outgoing thread stopped" );
}

} // namespace Stratus

} // namespace Agape
