#ifndef AGAPE_UI_VR_TIME_H
#define AGAPE_UI_VR_TIME_H

#include "Actors/NativeActors/NativeActor.h"

namespace Agape
{

namespace Carlo
{
class FunctionDispatcher;
} // namespace Carlo

namespace Clocks
{
class VRTime;
} // namespace Clocks

namespace Linda2
{
class TupleRouter;
} // namespace Linda2

using namespace Linda2;

class String;
class Terminal;
class Value;
class WindowManager;

namespace UI
{

class VRTime : public Actors::Native
{
    friend Clocks::VRTime;

public:
    VRTime( WindowManager& windowManager,
            const String& windowName,
            TupleRouter& tupleRouter,
            FunctionDispatcher& functionDispatcher );
    virtual ~VRTime();

    void doRegister();
    void draw();
    
    void start();
    void stop();

    bool haveTime();
    void waitForTime();

    virtual bool accept( Tuple& tuple );
    virtual bool perform( Value& returnValue,
                          const String& name,
                          Map< String, Value* > arguments,
                          const String& caller );

private:
    TupleRouter& m_tupleRouter;
    FunctionDispatcher& m_functionDispatcher;
    Terminal* m_terminal;

    bool m_registered;
    bool m_haveTime;

    bool m_running;

    long long m_secsSinceEpoch;
};

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_VR_TIME_H
