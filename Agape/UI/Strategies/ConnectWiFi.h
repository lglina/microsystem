#ifndef AGAPE_UI_STRATEGIES_CONNECT_WIFI_H
#define AGAPE_UI_STRATEGIES_CONNECT_WIFI_H

#include "UI/Strategy.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class InputDevice;
class Line;
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

class ConnectWiFi : public Strategy
{
public:
    ConnectWiFi( InputDevice& inputDevice,
                 WindowManager& windowManager,
                 const String& windowName,
                 Line& line,
                 Timers::Factory& timerFactory,
                 Dialogue& dialogue );
    ~ConnectWiFi();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        none,
        list,
        scanning,
        network,
        password,
        pending,
        success,
        error
    };

    void drawBackground();
    void drawList();
    void drawScanning();
    void drawNetwork();
    void drawPassword();
    void drawPendingScan();
    void drawPendingConnect();
    void drawSuccess();
    void drawError();
    void hideDialogue();

    void closeForm();

    void deleteNetwork();
    void setNetwork();
    void setPassword();
    void connect();

    InputDevice& m_inputDevice;
    WindowManager& m_windowManager;
    String m_windowName;
    Line& m_line;
    Dialogue& m_dialogue;

    Timer* m_timer;

    enum State m_state;
    bool m_completed;
    Value m_parameters;
    Value m_returnParameters;

    Terminal* m_terminal;
    Forms::Form* m_currentForm;

    bool m_addOnly;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_STRATEGIES_CONNECT_WIFI_H
