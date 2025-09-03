#ifndef AGAPE_STRATEGIES_PHONEBOOK_H
#define AGAPE_STRATEGIES_PHONEBOOK_H

#include "UI/Strategy.h"
#include "String.h"

namespace Agape
{

class Dialogue;
class InputDevice;
class Phonebook;
class Terminal;
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

class Phonebook : public Strategy
{
public:
    Phonebook( WindowManager& windowManager,
               const String& windowName,
               InputDevice& inputDevice,
               Agape::Phonebook& phonebook,
               Dialogue& dialogue );

    ~Phonebook();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        select,
        add
    };

    void drawBackground();
    void drawSelectForm();
    void drawAddForm();

    void closeForm();

    void addEntry();
    void deleteEntry();
    void setDefaultEntry();

    WindowManager& m_windowManager;
    String m_windowName;
    InputDevice& m_inputDevice;
    Agape::Phonebook& m_phonebook;
    Dialogue& m_dialogue;

    enum State m_state;

    bool m_completed;

    Forms::Form* m_currentForm;

    Terminal* m_terminal;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_PHONEBOOK_H
