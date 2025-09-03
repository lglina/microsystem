#ifndef AGAPE_STRATEGIES_ENTER_KEY_H
#define AGAPE_STRATEGIES_ENTER_KEY_H

#include "UI/Strategy.h"
#include "Value.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class ConfigurationStore;
class InputDevice;
class KeyUtilities;
class String;
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

class EnterKey : public Strategy
{
public:
    static const char* kAccount;
    static const char* kDevice;

    EnterKey( WindowManager& windowManager,
              const String& windowName,
              InputDevice& inputDevice,
              KeyUtilities& keyUtilities,
              ConfigurationStore& configurationStore,
              Dialogue& dialogue,
              Timers::Factory& timerFactory );

    ~EnterKey();

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
    void drawEnterKeyForm();
    void drawSuccess();
    void drawError();
    void hideDialogue();

    void closeForm();

    bool getKeyFromForm( char* key );
    void setKey( char* key );

    void autocompleteKeyword();
    
    WindowManager& m_windowManager;
    String m_windowName;
    InputDevice& m_inputDevice;
    KeyUtilities& m_keyUtilities;
    ConfigurationStore& m_configurationStore;
    Dialogue& m_dialogue;

    Timer* m_timer;

    Value m_parameters;
    enum State m_state;
    bool m_completed;
    Value m_returnParameters;

    Forms::Form* m_currentForm;

    Terminal* m_terminal;
};

} // namespace UI

} // namespace Strategies

} // namespace Agape

#endif // AGAPE_STRATEGIES_ENTER_KEY_H
