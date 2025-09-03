#ifndef AGAPE_LINDA2_PROMISE_H
#define AGAPE_LINDA2_PROMISE_H

#include "Value.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

namespace Linda2
{

class TupleRouter;

/// @brief Use a familiar future/promise structure to handle the common case
/// where a tuple is routed and we want to wait for a response to be recieved
/// from a distant actor before proceeding. First, create a promise, then
/// get a linked future via getFuture(). Next, route a tuple then call get() on
/// the future to poll the associated TupleHandler. When a response tuple(s)
/// is received, the caller's accept() handler (inherited from Actor) will
/// set the promise, allowing the future to return a success indication and,
/// optionally, an arbitrary value.
class Promise
{
public:
    class Future
    {
    public:
        Future( Promise& promise );

        bool get( Value& value );
        bool get();
    
    private:
        Promise& m_promise;
    };

    Promise();
    Promise( TupleRouter* tupleRouter,
             Timers::Factory* timerFactory );
    
    Future getFuture();

    bool set( const Value& successValue, const Value& returnValue );
    bool set( const Value& successValue );
    void set();

private:
    TupleRouter* m_tupleRouter;
    Timers::Factory* m_timerFactory;

    bool m_set;
    bool m_success;
    Value m_value;
};

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_PROMISE_H
