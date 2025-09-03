#ifndef AGAPE_WORLD_LOADERS_LINDA2_RESPONDER_H
#define AGAPE_WORLD_LOADERS_LINDA2_RESPONDER_H

#include "Actors/NativeActors/NativeActor.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace Linda2
{
class Tuple;
class TupleRouter;
} // namespace Linda2

namespace World
{
    class Metadata;
} // namespace World

using namespace World;

namespace WorldLoaders
{

class Factory;

class Linda2Responder : public Actors::Native
{
public:
    Linda2Responder( TupleRouter& tupleRouter, WorldLoaders::Factory& worldLoaderFactory );
    virtual ~Linda2Responder();

    virtual bool accept( Tuple& tuple );

private:
    void _create( const Tuple& tuple );
    void _join( const Tuple& tuple );
    void _load( const Tuple& tuple );
    void _loadJoined( const Tuple& tuple );
    void _loadTeleports( const Tuple& tuple );
    void _createTeleport( const Tuple& tuple );
    void _deleteTeleport( const Tuple& tuple );
    void _loadWorldSummaries( const Tuple& tuple );
    void _loadUniverseStats( const Tuple& tuple );

    TupleRouter& m_tupleRouter;
    WorldLoaders::Factory& m_worldLoaderFactory;
};

} // namespace WorldLoaders

} // namespace Agape

#endif // AGAPE_WORLD_LOADERS_LINDA2_RESPONDER_H
