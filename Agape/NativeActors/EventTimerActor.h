#ifndef AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_EVENT_TIMER_H
#define AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_EVENT_TIMER_H

#include "Actors/NativeActors/NativeActor.h"
#include "Runnable.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class Timer;

namespace Linda2
{

class TupleRouter;
class Tuple;

namespace Actors
{

namespace NativeActors
{

class EventTimer : public Actors::Native, public Runnable
{
public:
    EventTimer( Timers::Factory& timerFactory, TupleRouter& tupleRouter );
    virtual ~EventTimer();

    virtual bool accept( Tuple& tuple );

    virtual void run();

private:
    void sendTuple();

    TupleRouter& m_tupleRouter;

    Timer* m_timer;

    double m_lastS;
    double m_lastMs;
};

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_EVENT_TIMER_H
