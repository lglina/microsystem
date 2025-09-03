#ifndef AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_PLATFORM_H
#define AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_PLATFORM_H

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

class Platform;

namespace Linda2
{

namespace Actors
{

namespace NativeActors
{

class Platform : public Actors::Native
{
public:
    Platform( Carlo::FunctionDispatcher& functionDispatcher,
              Agape::Platform& platform );
    ~Platform();

    virtual bool perform( Value& returnValue,
                          const String& name,
                          Map< String, Value* > arguments,
                          const String& caller );

private:
    Carlo::FunctionDispatcher& m_functionDispatcher;
    Agape::Platform& m_platform;
};

} // namespace NativeActors

} // namespace Actors

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_ACTORS_NATIVE_ACTORS_PLATFORM_H
