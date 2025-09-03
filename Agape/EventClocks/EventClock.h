#ifndef AGAPE_EVENT_CLOCK_H
#define AGAPE_EVENT_CLOCK_H

#include "Runnable.h"

namespace Agape
{

namespace Linda2
{
class TupleRouter;
} // namespace Linda2

using namespace Linda2;

class EventClock : public Runnable
{
public:
    EventClock( TupleRouter& tupleRouter ) :
      m_tupleRouter( tupleRouter )
    {
    }
    
    virtual ~EventClock() {}

    virtual void run() = 0;

protected:
    TupleRouter& m_tupleRouter;
};

} // namespace Agape

#endif // AGAPE_EVENT_CLOCK_H
