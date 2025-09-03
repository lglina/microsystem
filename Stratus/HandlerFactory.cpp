#include "AssetLoaders/Factories/MongoAssetLoaderFactory.h"
#include "AssetLoaders/Linda2AssetLoaderResponder.h"
#include "Clocks/Clock.h"
#include "Clocks/CClock.h"
#include "Encryptors/AES/AESEncryptor.h"
#include "Encryptors/Factories/AESEncryptorFactory.h"
#include "Encryptors/Factories/EncryptorsFactory.h"
#include "Encryptors/SHA256/SHA256Hash.h"
#include "EntropySources/DevRandom.h"
#include "Network/WebSocketsConnection.h"
#include "PresenceLoaders/Factories/SharedPresenceLoaderFactory.h"
#include "PresenceLoaders/Linda2PresenceLoaderResponder.h"
#include "PresenceLoaders/SharedPresenceStore.h"
#include "SceneLoaders/Factories/MongoSceneLoaderFactory.h"
#include "SceneLoaders/Linda2SceneLoaderResponder.h"
#include "TelegramLoaders/Factories/MongoTelegramLoaderFactory.h"
#include "TelegramLoaders/Linda2TelegramLoaderResponder.h"
#include "Timers/Factories/CTimerFactory.h"
#include "TupleFilters/StratusTupleFilter.h"
#include "TupleFilters/TupleFilter.h"
#include "TupleRoutes/QueueingTupleRoute.h"
#include "TupleRoutes/ReadableWritableTupleRoute.h"
#include "WorldLoaders/Factories/MongoWorldLoaderFactory.h"
#include "WorldLoaders/Linda2WorldLoaderResponder.h"
#include "Authenticator.h"
#include "Handler.h"
#include "HandlerFactory.h"
#include "Hydra.h"
#include "Inviter.h"
#include "KeyUtilities.h"
#include "PushNotifier.h"
#include "RWBuffer.h"
#include "TupleDispatcher.h"
#include "TupleRouter.h"
#include "TupleRoutingCriteria.h"
#include "WebSockets.h"

#include <sstream>

namespace Agape
{

namespace Stratus
{

HandlerFactory::HandlerFactory() :
  m_clientNumber( 0 )
{
}

Handler* HandlerFactory::makeHandler( WSTLSServer::connection_ptr connection, Hydra& hydra, PresenceLoaders::SharedPresenceStore& sharedPresenceStore )
{
    String clientID;
    std::ostringstream oss;
    oss << "Client" << m_clientNumber++;
    clientID = oss.str().c_str();

    EntropySource* entropySource( new EntropySources::DevRandom );
    Encryptors::Factories::AES* encryptorFactory( new Encryptors::Factories::AES( *entropySource ) );
    Encryptor* encryptor( new Encryptors::AES( *entropySource ) );
    Hash* hash( new Hashes::SHA256 );
    Linda2::TupleDispatcher* tupleDispatcher( new TupleDispatcher );
    Timers::Factory* timerFactory( new Timers::Factories::C );
    Clocks::C* _clock( new Clocks::C );
    Linda2::TupleRouter* tupleRouter( new TupleRouter( *tupleDispatcher, clientID, *timerFactory ) );
    KeyUtilities* keyUtilities( new KeyUtilities( *entropySource, *encryptorFactory, *hash ) );
    Authenticator* authenticator( new Authenticator( *tupleRouter, *keyUtilities ) );
    AssetLoaders::Factory* assetLoaderFactory( new AssetLoaders::Factories::Mongo( "Assets", true, *authenticator ) ); // true = encrypted names
    AssetLoaders::Linda2Responder* assetLoaderResponder( new AssetLoaders::Linda2Responder( *tupleRouter, *assetLoaderFactory, "Assets" ) );
    AssetLoaders::Factory* programAssetLoaderFactory( new AssetLoaders::Factories::Mongo( "Programs", true, *authenticator ) ); // true = encrypted names
    AssetLoaders::Linda2Responder* programAssetLoaderResponder( new AssetLoaders::Linda2Responder( *tupleRouter, *programAssetLoaderFactory, "Programs" ) );
    AssetLoaders::Factory* telegramAssetLoaderFactory( new AssetLoaders::Factories::Mongo( "TelegramAssets", false, *authenticator ) ); // false = names not encrypted
    AssetLoaders::Linda2Responder* telegramAssetLoaderResponder( new AssetLoaders::Linda2Responder( *tupleRouter, *telegramAssetLoaderFactory, "TelegramAssets" ) );
    TelegramLoaders::Factory* telegramLoaderFactory( new TelegramLoaders::Factories::Mongo( *authenticator ) );
    TelegramLoaders::Linda2Responder* telegramLoaderResponder( new TelegramLoaders::Linda2Responder( *tupleRouter, *telegramLoaderFactory ) );
    PresenceLoaders::Factory* presenceLoaderFactory( new PresenceLoaders::Factories::Shared( sharedPresenceStore, *_clock, *authenticator ) );
    PresenceLoaders::Linda2Responder* presenceLoaderResponder( new PresenceLoaders::Linda2Responder( *tupleRouter, *presenceLoaderFactory ) );
    SceneLoaders::Factory* sceneLoaderFactory( new SceneLoaders::Factories::Mongo( *authenticator ) );
    SceneLoaders::Linda2Responder* sceneLoaderResponder( new SceneLoaders::Linda2Responder( *tupleRouter, *sceneLoaderFactory ) );
    WorldLoaders::Factory* worldLoaderFactory( new WorldLoaders::Factories::Mongo( *authenticator ) );
    WorldLoaders::Linda2Responder* worldLoaderResponder( new WorldLoaders::Linda2Responder( *tupleRouter, *worldLoaderFactory ) );
    Network::WebSocketsConnection* webSocketsConnection( new Network::WebSocketsConnection( connection ) );
    RWBuffer* rwBuffer( new RWBuffer( 1024, *webSocketsConnection ) );
    //RWBuffer* rwBuffer( new RWBuffer( 128, *webSocketsConnection ) );
    Linda2::TupleRoute* incomingTupleRoute( new Linda2::TupleRoutes::ReadableWritable( clientID, *rwBuffer ) );
    //Linda2::TupleRoute* incomingTupleRoute( new Linda2::TupleRoutes::ReadableWritable( handlerID + "<->H" + handlerID, *webSocketsConnection ) );
    Linda2::TupleRoutes::Queueing* hydraNearTupleRoute( new Linda2::TupleRoutes::Queueing( "Hydra" ) );
    Linda2::TupleRoutes::Queueing* hydraFarTupleRoute( new Linda2::TupleRoutes::Queueing( clientID ) );
    Inviter* inviter( new Inviter( *tupleRouter, *authenticator ) );
    PushNotifier* pushNotifier( new PushNotifier( *tupleRouter, *authenticator, sharedPresenceStore, *telegramLoaderFactory, *webSocketsConnection ) );

    hydraNearTupleRoute->setPartner( hydraFarTupleRoute );
    hydraFarTupleRoute->setPartner( hydraNearTupleRoute );

    hydra.addRoute( hydraFarTupleRoute );

    tupleRouter->setMyID( "Stratus" );

    tupleRouter->addRoute( incomingTupleRoute, false );
    tupleRouter->addRoute( hydraNearTupleRoute, true ); // Default route.

    // Subscribe the client to Time updates so the client will start receiving
    // them even before the client subscribes itself (keeping the connection
    // alive in the case of an idle tab reconnect from Tela).
    TupleRoutingCriteria timeRoutingCriteria;
    timeRoutingCriteria.m_types.push_back( new Value( _Time ) );
    incomingTupleRoute->addRoutingCriteria( timeRoutingCriteria );
    hydraFarTupleRoute->addRoutingCriteria( timeRoutingCriteria );

    Linda2::TupleFilter* tupleFilter( new TupleFilters::Stratus( *authenticator ) );
    tupleRouter->setTupleFilter( tupleFilter );

    return new Handler( entropySource,
                        encryptorFactory,
                        encryptor,
                        hash,
                        keyUtilities,
                        authenticator,
                        assetLoaderFactory,
                        assetLoaderResponder,
                        programAssetLoaderFactory,
                        programAssetLoaderResponder,
                        telegramAssetLoaderFactory,
                        telegramAssetLoaderResponder,
                        telegramLoaderFactory,
                        telegramLoaderResponder,
                        presenceLoaderFactory,
                        presenceLoaderResponder,
                        sceneLoaderFactory,
                        sceneLoaderResponder,
                        worldLoaderFactory,
                        worldLoaderResponder,
                        tupleDispatcher,
                        tupleFilter,
                        tupleRouter,
                        webSocketsConnection,
                        rwBuffer,
                        incomingTupleRoute,
                        hydra,
                        hydraNearTupleRoute,
                        hydraFarTupleRoute,
                        timerFactory,
                        _clock,
                        inviter,
                        pushNotifier );
}

} // namespace Stratus

} // namespace Agape
