#ifndef AGAPE_STRATUS_HANDLER_H
#define AGAPE_STRATUS_HANDLER_H

#include <memory>
#include <mutex>
#include <thread>

namespace Agape
{

namespace AssetLoaders
{
class Factory;
class Linda2Responder;
} // namespace AssetLoaders

namespace Encryptors
{
class Factory;
} // namespace Encryptors

namespace Linda2
{
namespace TupleRoutes
{
class Queueing;
} // namespace TupleRoutes
class TupleDispatcher;
class TupleFilter;
class TupleRoute;
class TupleRouter;
} // namespace Linda2

namespace Network
{
class WebSocketsConnection;
} // namespace Network

namespace PresenceLoaders
{
class Factory;
class Linda2Responder;
} // namespace PresenceLoaders

namespace SceneLoaders
{
class Factory;
class Linda2Responder;
} // namespace SceneLoaders

namespace TelegramLoaders
{
class Factory;
class Linda2Responder;
} // namespace TelegramLoaders

namespace Timers
{
class Factory;
} // namespace Timers

namespace WorldLoaders
{
class Factory;
class Linda2Responder;
} // namespace WorldLoaders

class Clock;
class Encryptor;
class EntropySource;
class Hash;
class KeyUtilities;
class RWBuffer;

namespace Stratus
{

class Authenticator;
class Hydra;
class Inviter;
class PushNotifier;

class Handler
{
public:
    Handler( EntropySource* entropySource,
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
             PushNotifier* pushNotifier
    );

    ~Handler();

    void handle();

    bool stopped();

private:
    void _handleIncoming();
    void _handleOutgoing();

    EntropySource* m_entropySource;
    Encryptors::Factory* m_encryptorFactory;
    Encryptor* m_encryptor;
    Hash* m_hash;
    KeyUtilities* m_keyUtilities;
    Authenticator* m_authenticator;
    AssetLoaders::Factory* m_assetLoaderFactory;
    AssetLoaders::Linda2Responder* m_assetLoaderResponder;
    AssetLoaders::Factory* m_programAssetLoaderFactory;
    AssetLoaders::Linda2Responder* m_programAssetLoaderResponder;
    AssetLoaders::Factory* m_telegramAssetLoaderFactory;
    AssetLoaders::Linda2Responder* m_telegramAssetLoaderResponder;
    TelegramLoaders::Factory* m_telegramLoaderFactory;
    TelegramLoaders::Linda2Responder* m_telegramLoaderResponder;
    PresenceLoaders::Factory* m_presenceLoaderFactory;
    PresenceLoaders::Linda2Responder* m_presenceLoaderResponder;
    SceneLoaders::Factory* m_sceneLoaderFactory;
    SceneLoaders::Linda2Responder* m_sceneLoaderResponder;
    WorldLoaders::Factory* m_worldLoaderFactory;
    WorldLoaders::Linda2Responder* m_worldLoaderResponder;
    Linda2::TupleDispatcher* m_tupleDispatcher;
    Linda2::TupleFilter* m_tupleFilter;
    Linda2::TupleRouter* m_tupleRouter;
    Network::WebSocketsConnection* m_webSocketsConnection;
    RWBuffer* m_rwBuffer;
    Linda2::TupleRoute* m_incomingTupleRoute;
    Hydra& m_hydra;
    Linda2::TupleRoutes::Queueing* m_hydraNearTupleRoute;
    Linda2::TupleRoutes::Queueing* m_hydraFarTupleRoute;
    Timers::Factory* m_timerFactory;
    Agape::Clock* m_clock;
    Inviter* m_inviter;
    PushNotifier* m_pushNotifier;

    std::unique_ptr< std::thread > m_incomingTuplesThread;
    std::unique_ptr< std::thread > m_outgoingTuplesThread;

    std::mutex m_mutex;

    bool m_stopping;
    bool m_stopped;
};

} // namespace Stratus

} // namespace Agape

#endif // AGAPE_STRATUS_HANDLER_H
