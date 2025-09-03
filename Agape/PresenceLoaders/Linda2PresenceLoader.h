#ifndef AGAPE_PRESENCE_LOADERS_LINDA2_H
#define AGAPE_PRESENCE_LOADERS_LINDA2_H

#include "Actors/NativeActors/NativeActor.h"
#include "World/ScenePresence.h"
#include "Collections.h"
#include "PresenceLoader.h"
#include "PresenceRequest.h"
#include "Promise.h"
#include "TupleRoutingCriteria.h"

using namespace Agape::Linda2;
using namespace Agape::World;

namespace Agape
{

namespace Linda2
{
class Tuple;
class TupleRouter;
} // namespace Linda2

namespace Timers
{
class Factory;
} // namespace Timers

namespace World
{
class Coordinates;
} // namespace World

namespace PresenceLoaders
{

class Linda2 : public PresenceLoader, public Actors::Native
{
public:
    Linda2( const World::Coordinates& coordinates,
            bool receiveRequests,
            TupleRouter& tupleRouter,
            Timers::Factory& timerFactory );
    virtual ~Linda2();

    virtual bool load( Vector< ScenePresence >& scenePresences );

    virtual bool loadWorld( Vector< ScenePresence >& worldPresences );

    virtual bool request( const Vector< PresenceRequest >& requests );
    virtual Vector< PresenceRequest > getUpdates();

    virtual bool overflowed();

    virtual bool accept( Tuple& tuple );

private:
    bool m_receiveRequests;
    TupleRouter& m_tupleRouter;
    Timers::Factory& m_timerFactory;

    bool m_isLoading;
    int m_currentItem;
    int m_totalItems;

    Promise m_presenceLoadResponse;
    Promise m_presenceLoadWorldResponse;

    TupleRoutingCriteria m_tupleRoutingCriteria;

    Vector< ScenePresence >* m_presences;

    Vector< PresenceRequest > m_updates;

    bool m_overflowed;
};

} // namespace PresenceLoaders

} // namespace Agape

#endif // AGAPE_PRESENCE_LOADERS_LINDA2_H
