#ifndef AGAPE_WORLD_LOADERS_LINDA2_H
#define AGAPE_WORLD_LOADERS_LINDA2_H

#include "Actors/NativeActors/NativeActor.h"
#include "World/UniverseStats.h"
#include "World/WorldMetadata.h"
#include "World/WorldSummary.h"
#include "Collections.h"
#include "Promise.h"
#include "WorldLoader.h"
#include "String.h"

using namespace Agape::Linda2;

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

namespace WorldLoaders
{

class Linda2 : public WorldLoader, public Actors::Native
{
public:
    Linda2( TupleRouter& tupleRouter,
            Timers::Factory& timerFactory );
    virtual ~Linda2();

    virtual bool create( const World::Metadata& metadata, String& reason );
    virtual bool join( World::Metadata& metadata, String& reason );
    virtual bool load( World::Metadata& metadata, String& reason );

    virtual bool loadJoinedWorlds( Vector< World::Metadata >& joinedWorlds, bool allDevices, String& reason );

    virtual bool loadTeleports( Vector< World::Teleport >& teleports, bool allDevices, String& reason );
    virtual bool createTeleport( World::Teleport& teleport, String& reason );
    virtual bool deleteTeleport( World::Teleport& teleport, String& reason );

    virtual bool loadWorldSummaries( Vector< World::Summary >& worldSummaries, int from, int size, String& reason );

    virtual bool loadUniverseStats( World::UniverseStats& universeStats, String& reason );

    virtual bool accept( Tuple& tuple );

private:
    TupleRouter& m_tupleRouter;
    Timers::Factory& m_timerFactory;

    Promise m_worldCreateResponse;
    Promise m_worldJoinResponse;
    Promise m_worldLoadResponse;
    Promise m_worldLoadJoinedResponse;
    Promise m_worldLoadTeleportsResponse;
    Promise m_worldCreateTeleportResponse;
    Promise m_worldDeleteTeleportResponse;
    Promise m_worldLoadWorldSummariesResponse;
    Promise m_worldLoadUniverseStatsResponse;
    
    bool m_isLoading;

    World::Metadata m_metadata;

    int m_currentJoinedWorld;
    int m_totalJoinedWorlds;
    Vector< World::Metadata >* m_joinedWorlds;

    int m_currentTeleport;
    int m_totalTeleports;
    Vector< World::Teleport >* m_teleports;

    Vector< World::Summary >* m_worldSummaries;

    World::UniverseStats* m_universeStats;
};

} // namespace WorldLoaders

} // namespace Agape

#endif // AGAPE_WORLD_LOADERS_LINDA2_H
