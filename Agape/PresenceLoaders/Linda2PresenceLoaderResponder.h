#ifndef AGAPE_PRESENCE_LOADERS_LINDA2_RESPONDER_H
#define AGAPE_PRESENCE_LOADERS_LINDA2_RESPONDER_H

#include "Actors/NativeActors/NativeActor.h"
#include "World/ScenePresence.h"
#include "Collections.h"
#include "PresenceRequest.h"
#include "String.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace Linda2
{
class TupleRouter;
} // namespace Linda2

class PresenceLoader;

namespace PresenceLoaders
{

class Factory;

class Linda2Responder : public Actors::Native
{
public:
    Linda2Responder( TupleRouter& tupleRouter, PresenceLoaders::Factory& presenceLoaderFactory );
    virtual ~Linda2Responder();

    virtual bool accept( Tuple& tuple );

    void reset();

    void forceDepart();

private:
    void loadPresences( const Tuple& tuple );
    void loadWorldPresences( const Tuple& tuple );

    void handleRequest( const Tuple& tuple );

    TupleRouter& m_tupleRouter;
    PresenceLoaders::Factory& m_presenceLoaderFactory;

    Map< String, PresenceRequest > m_latestPresenceRequests;
};

} // namespace PresenceLoaders

} // namespace Agape

#endif // AGAPE_PRESENCE_LOADERS_LINDA2_RESPONDER_H
