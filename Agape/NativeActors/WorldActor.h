#ifndef AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_WORLD_H
#define AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_WORLD_H

#include "Actors/NativeActors/NativeActor.h"

namespace Agape
{

namespace Carlo
{
class FunctionDispatcher;
} // namespace Carlo

namespace World
{
class Compositor;
class Coordinates;
} // namespace World

using namespace Agape::World;

class Snowflake;
class Worldbook;

namespace Linda2
{

class TupleRouter;

namespace Actors
{

namespace NativeActors
{

class World : public Actors::Native
{
public:
    World( TupleRouter& tupleRouter,
           FunctionDispatcher& functionDispatcher,
           Compositor& compositor,
           Agape::Snowflake& snowflake,
           Coordinates& coordinates,
           Worldbook& worldbook );
    virtual ~World();

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
    Agape::Snowflake& m_snowflake;
    Coordinates& m_coordinates;
    Worldbook& m_worldbook;

    int m_prevX;
    int m_prevY;
};

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_WORLD_H
