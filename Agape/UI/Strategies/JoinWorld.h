#ifndef AGAPE_STRATEGIES_JOIN_WORLD_H
#define AGAPE_STRATEGIES_JOIN_WORLD_H

#include "UI/Strategy.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

namespace World
{
class Utilities;
} // namespace World

namespace WorldLoaders
{
class Factory;
} // namespace WorldLoaders

class InputDevice;
class Terminal;
class Timer;
class WindowManager;
class Worldbook;

namespace UI
{

namespace Forms
{
class Form;
} // namespace Forms

class Dialogue;

namespace Strategies
{

using namespace Agape::World;

class JoinWorld : public Strategy
{
public:
    JoinWorld( InputDevice& inputDevice,
               WindowManager& windowManager,
               const String& windowName,
               Worldbook& worldbook,
               World::Utilities& worldUtilities,
               WorldLoaders::Factory& worldLoaderFactory,
               Timers::Factory& timer,
               Dialogue& dialogue );
    ~JoinWorld();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        none,
        enterKey,
        success,
        error
    };

    void drawBackground();
    void drawEnterKey();
    void drawPending();
    void drawSuccess();
    void drawError();
    void drawKeyError();
    void hideDialogue();

    void closeForm();

    bool getKey( char* key );
    bool joinWorld( User& user, Metadata& plainMetadata );

    void autocompleteKeyword();

    InputDevice& m_inputDevice;
    WindowManager& m_windowManager;
    String m_windowName;
    Worldbook& m_worldbook;
    World::Utilities& m_worldUtilities;
    WorldLoaders::Factory& m_worldLoaderFactory;
    Timer* m_timer;
    Dialogue& m_dialogue;

    Value m_parameters;
    enum State m_state;
    bool m_completed;
    Value m_returnParameters;

    Terminal* m_terminal;
    Forms::Form* m_currentForm;

    char m_key[16];
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_JOIN_WORLD_H
