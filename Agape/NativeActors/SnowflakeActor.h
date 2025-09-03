#ifndef AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_SNOWFLAKE_H
#define AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_SNOWFLAKE_H

#include "Actors/NativeActors/NativeActor.h"
#include "Collections.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace Carlo
{
class FunctionDispatcher;
} // namespace Carlo

namespace Linda2
{

namespace Actors
{

namespace NativeActors
{

class Snowflake : public Actors::Native
{
public:
    Snowflake( Carlo::FunctionDispatcher& functionDispatcher );
    ~Snowflake();

    virtual bool perform( Value& returnValue,
                          const String& name,
                          Map< String, Value* > arguments,
                          const String& caller );

private:
    Carlo::FunctionDispatcher& m_functionDispatcher;
};

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_SNOWFLAKE_H
