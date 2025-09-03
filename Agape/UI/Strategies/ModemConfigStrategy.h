#ifndef AGAPE_STRATEGIES_MODEM_CONFIG_H
#define AGAPE_STRATEGIES_MODEM_CONFIG_H

#include "Lines/Line.h"
#include "UI/Strategy.h"
#include "Collections.h"
#include "String.h"

namespace Agape
{

class InputDevice;
class Value;
class WindowManager;

namespace UI
{

namespace Forms
{
class Form;
} // namespace Forms

namespace Strategies
{

class ModemConfig : public Strategy
{
public:
    ModemConfig( InputDevice& inputDevice,
                 WindowManager& windowManager,
                 const String& windowName,
                 Line& line );
    ~ModemConfig();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    void drawConfigureModem();

    void closeForm();

    void configureModem();

    InputDevice& m_inputDevice;
    WindowManager& m_windowManager;
    String m_windowName;
    Line& m_line;

    bool m_completed;
    String m_nextStrategy;
    
    Vector< Line::ConfigOption > m_modemConfigOptions;

    Forms::Form* m_currentForm;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_MODEM_CONFIG_H
