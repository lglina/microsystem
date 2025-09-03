#ifndef AGAPE_TIMER_H
#define AGAPE_TIMER_H

namespace Agape
{

class Timer
{
public:
    virtual ~Timer() {}
    
    virtual long ms() = 0;
    virtual long us() { return ms() * 1000; }
    virtual void reset() = 0;

    // If supported by implementation, a blocking sleep that doesn't busy wait,
    // otherwise just returns immediately.
    virtual void usleep( long us ) {};
};

} // namespace Agape

#endif // AGAPE_TIMER_H
