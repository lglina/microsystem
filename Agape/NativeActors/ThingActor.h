#ifndef AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_THING_H
#define AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_THING_H

#include "Actors/NativeActors/NativeActor.h"
#include "Collections.h"
#include "Value.h"

namespace Agape
{

namespace Carlo
{
class FunctionDispatcher;
} // namespace Carlo

namespace World
{
class Compositor;
} // namespace World

using namespace World;

namespace Linda2
{

class TupleRouter;

namespace Actors
{

namespace NativeActors
{

class Thing : public Actors::Native
{
public:
    Thing( TupleRouter& tupleRouter,
           FunctionDispatcher& functionDispatcher,
           Compositor& compositor );
    virtual ~Thing();

    virtual bool accept( Tuple& tuple );
    
    virtual bool perform( Value& returnValue,
                          const String& name,
                          Map< String, Value* > arguments,
                          const String& caller );

    virtual bool getPersistableValue( Value& value,
                                      const String& name,
                                      const String& caller );

private:
    TupleRouter& m_tupleRouter;
    FunctionDispatcher& m_functionDispatcher;
    Compositor& m_compositor;
};

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_THING_H
