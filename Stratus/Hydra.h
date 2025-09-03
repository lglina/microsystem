#ifndef AGAPE_STRATUS_HYDRA_H
#define AGAPE_STRATUS_HYDRA_H

#include "Timers/Factories/CTimerFactory.h"
#include "TupleDispatcher.h"
#include "TupleRouter.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

using namespace Agape::Linda2;

namespace Agape
{

namespace Linda2
{
class TupleRoute;
} // namespace Linda2

namespace Stratus
{

class Hydra
{
public:
    Hydra();
    ~Hydra();

    void addRoute( TupleRoute* route );
    void removeRoute( TupleRoute* route );

    void signalIncoming();

private:
    void route();

    Timers::Factories::C m_timerFactory;

    TupleDispatcher m_tupleDispatcher;
    TupleRouter m_tupleRouter;

    std::unique_ptr< std::thread > m_routingThread;

    std::mutex m_mutex;
    std::condition_variable m_incomingTuple;

    bool m_stopping;
};

} // namespace Stratus

} // namespace Agape

#endif // AGAPE_STRATUS_HYDRA_H
