#ifndef AGAPE_STRATEGIES_CONNECT_H
#define AGAPE_STRATEGIES_CONNECT_H

#include "UI/Strategy.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class ConfigurationStore;
class InputDevice;
class Line;
class Phonebook;
class Terminal;
class Timer;
class WindowManager;

namespace UI
{

class Dialogue;

namespace Strategies
{

class Connect : public Strategy
{
public:
    Connect( InputDevice& inputDevice,
             WindowManager& windowManager,
             const String& windowName,
             Phonebook& phonebook,
             ConfigurationStore& configurationStore,
             Line& line,
             Timers::Factory& timerFactory,
             Dialogue& dialogue );
    ~Connect();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        none,
        readyPending,
        carrierPending,
        success,
        message,
        error
    };

    void drawReadyPending();
    void drawCarrierPending();
    void drawSuccess();
    void drawPhonebookError();
    void drawReadyError();
    void drawCarrierError();
    void drawKeysError();
    void hideDialogue();

    bool dial();

    InputDevice& m_inputDevice;
    WindowManager& m_windowManager;
    String m_windowName;
    Phonebook& m_phonebook;
    ConfigurationStore& m_configurationStore;
    Line& m_line;
    Timer* m_timer;
    Dialogue& m_dialogue;

    enum State m_state;
    bool m_calling;
    Value m_callingParameters;
    String m_nextStrategy;
    bool m_completed;
    Value m_returnParameters;

    Terminal* m_terminal;

    bool m_nonInteractive;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_CONNECT_H
