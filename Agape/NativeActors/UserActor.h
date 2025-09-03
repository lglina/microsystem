#ifndef AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_USER_H
#define AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_USER_H

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
class User;
} // namespace World

using namespace World;

namespace Linda2
{

class TupleRouter;

namespace Actors
{

namespace NativeActors
{

class User : public Actors::Native
{
public:
    User( TupleRouter& tupleRouter,
          FunctionDispatcher& functionDispatcher,
          Compositor& compositor,
          World::User& worldUser );
    virtual ~User();

    virtual bool accept( Tuple& tuple );
    virtual bool perform( Value& returnValue,
                          const String& name,
                          Map< String, Value* > arguments,
                          const String& caller );

private:
    TupleRouter& m_tupleRouter;
    FunctionDispatcher& m_functionDispatcher;
    Compositor& m_compositor;
    World::User& m_worldUser;
};

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_USER_H
