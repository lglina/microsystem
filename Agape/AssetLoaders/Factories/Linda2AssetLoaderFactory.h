#ifndef AGAPE_ASSET_LOADERS_FACTORIES_LINDA2_H
#define AGAPE_ASSET_LOADERS_FACTORIES_LINDA2_H

#include "AssetLoadersFactory.h"
#include "String.h"

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

class AssetLoader;

namespace AssetLoaders
{

namespace Factories
{

class Linda2 : public Factory
{
public:
    Linda2( TupleRouter& tupleRouter,
            Timers::Factory& timerFactory,
            const String& collectionName );

    virtual AssetLoader* makeLoader( const World::Coordinates& coordinates, const String& name );

private:
    TupleRouter& m_tupleRouter;
    Timers::Factory& m_timerFactory;
    String m_collectionName;
};

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_FACTORIES_LINDA2_H
