#ifndef AGAPE_STRATEGIES_CREATE_WORLD_H
#define AGAPE_STRATEGIES_CREATE_WORLD_H

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
class Metadata;
class User;
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

class CreateWorld : public Strategy
{
public:
    CreateWorld( InputDevice& inputDevice,
                 WindowManager& windowManager,
                 const String& windowName,
                 World::Utilities& worldUtilities,
                 WorldLoaders::Factory& worldLoaderFactory,
                 Timers::Factory& timerFactory,
                 Dialogue& dialogue );
    ~CreateWorld();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        none,
        enterName,
        generateKey,
        enterKey,
        success,
        error
    };

    void drawBackground();
    void drawEnterName();
    void drawGenerateKey( const char* key );
    void drawEnterKey();
    void drawPending();
    void drawSuccess();
    void drawError();
    void drawKeyError();
    void hideDialogue();

    void closeForm();

    bool getKey( char* key );
    bool createWorld( User& user, Metadata& plainMetadata );
    
    void autocompleteKeyword();

    InputDevice& m_inputDevice;
    WindowManager& m_windowManager;
    String m_windowName;
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

    String m_name;
    char m_key[16];
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_CREATE_WORLD_H
