#include "AssetLoaders/Linda2AssetLoader.h"
#include "Timers/Factories/TimerFactory.h"
#include "World/WorldCoordinates.h"
#include "Linda2AssetLoaderFactory.h"
#include "String.h"
#include "TupleRouter.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace AssetLoaders
{

namespace Factories
{

Linda2::Linda2( TupleRouter& tupleRouter,
                Timers::Factory& timerFactory,
                const String& collectionName ) :
  m_tupleRouter( tupleRouter ),
  m_timerFactory( timerFactory ),
  m_collectionName( collectionName )
{
}

AssetLoader* Linda2::makeLoader( const World::Coordinates& coordinates, const String& name )
{
    return new AssetLoaders::Linda2( coordinates,
                                     name,
                                     m_tupleRouter,
                                     m_timerFactory,
                                     m_collectionName );
}

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape
