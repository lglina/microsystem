#ifndef AGAPE_UI_STRATEGIES_MENU_H
#define AGAPE_UI_STRATEGIES_MENU_H

#include "UI/Forms/Form.h"
#include "UI/Strategy.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

class Chooser;
class InputDevice;
class WindowManager;

namespace UI
{

namespace Strategies
{

class Menu : public Strategy
{
public:
    Menu( InputDevice& inputDevice,
          WindowManager& windowManager,
          const String& windowName,
          Chooser& chooser,
          const String& statusWindowName );
    ~Menu();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    void drawBackground();
    void drawMenu();
    void drawChooserState();

    void closeForm();

    InputDevice& m_inputDevice;
    WindowManager& m_windowManager;
    String m_windowName;
    Chooser& m_chooser;

    bool m_calling;
    bool m_completed;
    Value m_callingParameters;
    Value m_returnParameters;

    String m_nextStrategy;

    bool m_autoEnter;

    Forms::Form* m_currentForm;

    Terminal* m_terminal;
    Terminal* m_statusTerminal;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_STRATEGIES_MENU_H
