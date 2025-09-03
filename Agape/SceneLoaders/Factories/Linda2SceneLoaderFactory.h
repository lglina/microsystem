#ifndef AGAPE_SCENE_LOADERS_FACTORIES_LINDA2_H
#define AGAPE_SCENE_LOADERS_FACTORIES_LINDA2_H

#include "SceneLoadersFactory.h"

using namespace Agape::Linda2;

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

class SceneLoader;

namespace SceneLoaders
{

namespace Factories
{

class Linda2 : public Factory
{
public:
    Linda2( TupleRouter& tupleRouter,
            Timers::Factory& timerFactory,
            bool encrypted = false );

    virtual SceneLoader* makeLoader( const World::Coordinates& coordinates, bool receiveRequests );

private:
    TupleRouter& m_tupleRouter;
    Timers::Factory& m_timerFactory;
    bool m_encrypted;
};

} // namespace Factories

} // namespace SceneLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_FACTORIES_LINDA2_H
