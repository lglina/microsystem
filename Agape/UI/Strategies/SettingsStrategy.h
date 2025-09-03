#ifndef AGAPE_STRATEGIES_SETTINGS_H
#define AGAPE_STRATEGIES_SETTINGS_H

#include "UI/Strategy.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

class InputDevice;
class WindowManager;

namespace UI
{

namespace Forms
{
class Form;
} // namespace Forms

namespace Strategies
{

class Settings : public Strategy
{
public:
    Settings( InputDevice& inputDevice,
              WindowManager& windowManager,
              const String& windowName );
    ~Settings();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    void drawBackground();
    void drawMenu();

    void closeForm();

    InputDevice& m_inputDevice;
    WindowManager& m_windowManager;
    String m_windowName;

    bool m_completed;
    bool m_calling;
    String m_nextStrategy;
    Value m_callingParameters;

    Forms::Form* m_currentForm;

    Terminal* m_terminal;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_SETTINGS_H
