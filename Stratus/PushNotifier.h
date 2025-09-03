#ifndef AGAPE_STRATUS_PUSH_NOTIFIER_H
#define AGAPE_STRATUS_PUSH_NOTIFIER_H

#include "Actors/NativeActors/NativeActor.h"

namespace Agape
{

namespace Linda2
{
class Tuple;
class TupleRouter;
} // namespace Linda2

using namespace Linda2;

namespace Network
{
class WebSocketsConnection;
} // namespace Network

using namespace Network;

namespace PresenceLoaders
{
class SharedPresenceStore;
} // namespace PresenceLoaders

using namespace PresenceLoaders;

namespace TelegramLoaders
{
class Factory;
} // namespace TelegramLoaders

namespace Stratus
{

class Authenticator;

class PushNotifier : public Actors::Native
{
public:
    PushNotifier( TupleRouter& tupleRouter,
                  const Authenticator& authenticator,
                  const SharedPresenceStore& sharedPresenceStore,
                  TelegramLoaders::Factory& telegramLoaderFactory,
                  WebSocketsConnection& webSocketsConnection );
    virtual ~PushNotifier();

    virtual bool accept( Tuple& tuple );

private:
    bool isIdle();

    TupleRouter& m_tupleRouter;
    const Authenticator& m_authenticator;
    const SharedPresenceStore& m_sharedPresenceStore;
    TelegramLoaders::Factory& m_telegramLoaderFactory;
    WebSocketsConnection& m_webSocketsConnection;

    long long m_lastCheckedTelegrams;
    int m_lastUnreadTelegrams;
};

} // namespace Stratus

} // namespace Agape

#endif // AGAPE_STRATUS_PUSH_NOTIFIER_H
