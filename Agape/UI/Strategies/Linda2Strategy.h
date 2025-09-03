#ifndef AGAPE_STRATEGIES_LINDA2_H
#define AGAPE_STRATEGIES_LINDA2_H

#include "Actors/Actor.h"
#include "UI/Strategy.h"
#include "String.h"

namespace Agape
{

namespace Carlo
{
class FunctionDispatcher;
} // namespace Carlo

namespace Linda2
{
class Tuple;
class TupleRouter;
} // namespace Linda2

namespace World
{
class Compositor;
} // namespace World

class InputDevice;
class LiteStream;
class Terminal;
class WindowManager;
class Value;

using namespace Carlo;

namespace UI
{

namespace Forms
{
class Form;
} // namespace Forms

class Hotkeys;

namespace Strategies
{

class Linda2 : public Strategy, public Actor
{
public:
    Linda2( InputDevice& inputDevice,
            World::Compositor& compositor,
            WindowManager& windowManager,
            Hotkeys& hotkeys,
            TupleRouter& tupleRouter,
            FunctionDispatcher& functionDispatcher,
            const String& windowName );
    virtual ~Linda2();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

    virtual void str( LiteStream& stream, int indent );

    virtual bool accept( Tuple& tuple );
    virtual bool perform( Value& returnValue,
                          const String& name,
                          Map< String, Value* > arguments,
                          const String& caller );

    virtual String actorName() const;

private:
    void drawHotkeys();
    void drawImmediateHotkeys();
    void drawImmediateForm();
    void recall();
    void closeForm();

    void runImmediate();

    InputDevice& m_inputDevice;
    World::Compositor& m_compositor;
    WindowManager& m_windowManager;
    Hotkeys& m_hotkeys;
    TupleRouter& m_tupleRouter;
    FunctionDispatcher& m_functionDispatcher;
    String m_windowName;

    bool m_completed;

    bool m_active;
    bool m_localOnly;
    bool m_detail;
    bool m_pause;
    bool m_immediate;
    String m_immediateString;

    Terminal* m_terminal;

    Forms::Form* m_currentForm;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_LINDA2_H
