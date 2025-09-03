#include "SceneLoaders/Linda2SceneLoader.h"
#include "SceneLoaders/SceneLoader.h"
#include "Timers/Factories/TimerFactory.h"
#include "World/WorldCoordinates.h"
#include "Linda2SceneLoaderFactory.h"
#include "TupleRouter.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace SceneLoaders
{

namespace Factories
{

Linda2::Linda2( TupleRouter& tupleRouter,
                Timers::Factory& timerFactory,
                bool encrypted ) :
  m_tupleRouter( tupleRouter ),
  m_timerFactory( timerFactory ),
  m_encrypted( encrypted )
{
}

SceneLoader* Linda2::makeLoader( const World::Coordinates& coordinates, bool receiveRequests )
{
    return new SceneLoaders::Linda2( coordinates,
                                     receiveRequests,
                                     m_tupleRouter,
                                     m_timerFactory,
                                     m_encrypted );
}

} // namespace Factories

} // namespace SceneLoaders

} // namespace Agape
