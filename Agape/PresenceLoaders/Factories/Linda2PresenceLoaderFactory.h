#ifndef AGAPE_PRESENCE_LOADERS_FACTORIES_LINDA2_H
#define AGAPE_PRESENCE_LOADERS_FACTORIES_LINDA2_H

#include "PresenceLoadersFactory.h"

namespace Agape
{

namespace Linda2
{
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

class PresenceLoader;

namespace PresenceLoaders
{

namespace Factories
{

class Linda2 : public Factory
{
public:
    Linda2( TupleRouter& tupleRouter,
            Timers::Factory& timerFactory );

    virtual PresenceLoader* makeLoader( const World::Coordinates& coordinates, bool receiveRequests );

private:
    TupleRouter& m_tupleRouter;
    Timers::Factory& m_timerFactory;
};

} // namespace Factories

} // namespace PresenceLoaders

} // namespace Agape

#endif // AGAPE_PRESENCE_LOADERS_FACTORIES_LINDA2_H
