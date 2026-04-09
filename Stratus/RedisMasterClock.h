#ifndef AGAPE_STRATUS_CLOCK_H
#define AGAPE_STRATUS_CLOCK_H

#include "Timers/Factories/CTimerFactory.h"
#include "TupleRoutes/QueueingTupleRoute.h"
#include "TupleDispatcher.h"
#include "TupleRouter.h"

#include <hiredis/hiredis.h>

#include <chrono>

namespace Agape
{

namespace Stratus
{

using namespace Linda2;

using namespace std::chrono;

class MasterClock
{
public:
    MasterClock();
    ~MasterClock();
    
    void run();

private:
    Timers::Factories::C m_timerFactory;

    time_point< system_clock > m_lastTime;

    redisContext* m_redisContext;
};

} // namespace Stratus

} // namespace Agape

#endif // AGAPE_STRATUS_CLOCK_H
