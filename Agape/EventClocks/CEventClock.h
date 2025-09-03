#ifndef AGAPE_EVENT_CLOCKS_C_H
#define AGAPE_EVENT_CLOCKS_C_H

#include "EventClock.h"

#include <chrono>

namespace Agape
{

using namespace std::chrono;

namespace EventClocks
{

class C : public EventClock
{
public:
    C( TupleRouter& tupleRouter );
    
    virtual void run();

private:
    time_point< system_clock > m_lastTime;
};

} // namespace EventClocks

} // namespace Agape

#endif // AGAPE_EVENT_CLOCKS_C_H
