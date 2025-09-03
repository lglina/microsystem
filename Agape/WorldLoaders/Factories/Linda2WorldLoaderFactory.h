#ifndef AGAPE_WORLD_LOADERS_FACTORIES_LINDA2_H
#define AGAPE_WORLD_LOADERS_FACTORIES_LINDA2_H

#include "WorldLoadersFactory.h"

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

namespace WorldLoaders
{

namespace Factories
{

class Linda2 : public Factory
{
public:
    Linda2( TupleRouter& tupleRouter,
            Timers::Factory& timerFactory );

    virtual WorldLoader* makeLoader();

private:
    TupleRouter& m_tupleRouter;
    Timers::Factory& m_timerFactory;
};

} // namespace Factories

} // namespace WorldLoaders

} // namespace Agape

#endif // AGAPE_WORLD_LOADERS_FACTORIES_LINDA2_H
