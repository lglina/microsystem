#ifndef AGAPE_EVENT_CLOCKS_NULL_H
#define AGAPE_EVENT_CLOCKS_NULL_H

#include "EventClock.h"

namespace Agape
{

namespace EventClocks
{

class Null : public EventClock
{
public:
    Null( TupleRouter& tupleRouter );
    
    virtual void run();
};

} // namespace EventClocks

} // namespace Agape

#endif // AGAPE_EVENT_CLOCKS_NULL_H
