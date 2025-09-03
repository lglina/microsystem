#ifndef AGAPE_LINDA2_TUPLE_ROUTES_READABLE_WRITABLE_H
#define AGAPE_LINDA2_TUPLE_ROUTES_READABLE_WRITABLE_H

#include "ReadableWritable.h"
#include "String.h"
#include "TupleRoute.h"

namespace Agape
{

namespace Linda2
{

class Tuple;

namespace TupleRoutes
{

class ReadableWritable : public TupleRoute
{
public:
    ReadableWritable( const String& routeName, Agape::ReadableWritable& rw );

    // Returns true always, as ReadableWritable objects don't
    // indicate if any bytes are waiting.
    virtual bool haveIncoming();

    // Returns false immediately if no bytes can be read
    // (Tuple::fromReadableWritable tries a non-blocking read
    // on the first byte), else blocks until a whole tuple
    // is received.
    virtual bool receiveTuple( Tuple& tuple );

    virtual void run();

    virtual bool error() const;

private:
    virtual bool _sendTuple( const Tuple& tuple );

    Agape::ReadableWritable& m_rw;
};

}

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_TUPLE_ROUTES_READABLE_WRITABLE_H
