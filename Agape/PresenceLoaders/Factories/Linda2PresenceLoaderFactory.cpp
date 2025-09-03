#include "Linda2PresenceLoaderFactory.h"
#include "PresenceLoaders/Linda2PresenceLoader.h"
#include "PresenceLoaders/PresenceLoader.h"
#include "Timers/Factories/TimerFactory.h"
#include "World/WorldCoordinates.h"
#include "TupleRouter.h"

namespace Agape
{

namespace PresenceLoaders
{

namespace Factories
{

Linda2::Linda2( TupleRouter& tupleRouter,
                Timers::Factory& timerFactory ) :
  m_tupleRouter( tupleRouter ),
  m_timerFactory( timerFactory )
{
}

PresenceLoader* Linda2::makeLoader( const World::Coordinates& coordinates, bool receiveRequests )
{
    return new PresenceLoaders::Linda2( coordinates,
                                        receiveRequests,
                                        m_tupleRouter,
                                        m_timerFactory );
}

} // namespace Factories

} // namespace PresenceLoaders

} // namespace Agape
