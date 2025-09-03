#ifndef AGAPE_STRATUS_CLOCK_H
#define AGAPE_STRATUS_CLOCK_H

#include "Timers/Factories/CTimerFactory.h"
#include "TupleRoutes/QueueingTupleRoute.h"
#include "TupleDispatcher.h"
#include "TupleRouter.h"

#include <chrono>

namespace Agape
{

namespace Stratus
{

class Hydra;

using namespace Linda2;

using namespace std::chrono;

class MasterClock
{
public:
    MasterClock( Hydra& hydra );
    ~MasterClock();
    
    void run();

private:
    Hydra& m_hydra;
    TupleDispatcher m_tupleDispatcher;
    Timers::Factories::C m_timerFactory;
    TupleRouter m_tupleRouter;
    TupleRoutes::Queueing m_hydraNearTupleRoute;
    TupleRoutes::Queueing m_hydraFarTupleRoute;

    time_point< system_clock > m_lastTime;
};

} // namespace Stratus

} // namespace Agape

#endif // AGAPE_STRATUS_CLOCK_H
