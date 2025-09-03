#include "Timers/Factories/TimerFactory.h"
#include "WorldLoaders/Linda2WorldLoader.h"
#include "Linda2WorldLoaderFactory.h"
#include "TupleRouter.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace WorldLoaders
{

namespace Factories
{

Linda2::Linda2( TupleRouter& tupleRouter,
                Timers::Factory& timerFactory ) :
  m_tupleRouter( tupleRouter ),
  m_timerFactory( timerFactory )
{
}

WorldLoader* Linda2::makeLoader()
{
    return new WorldLoaders::Linda2( m_tupleRouter,
                                     m_timerFactory );
}

} // namespace Factories

} // namespace WorldLoaders

} // namespace Agape
