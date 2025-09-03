#ifndef AGAPE_UI_STRATEGIES_CHOOSE_AVATAR_H
#define AGAPE_UI_STRATEGIES_CHOOSE_AVATAR_H

#include "UI/Strategy.h"
#include "Value.h"
#include "String.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class EntropySource;
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

namespace Strategies
{

class ChooseAvatar : public Strategy
{
public:
    ChooseAvatar( InputDevice& inputDevice,
                  WindowManager& windowManager,
                  const String& windowName,
                  EntropySource& EntropySource,
                  Timers::Factory& timerFactory );
    ~ChooseAvatar();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        none,
        chooseAvatar,
        chooseName
    };

    void drawBackground();
    void drawChooseAvatar();
    void drawChooseName();

    void closeForm();

    void avatarWander();
    void switchAvatar();
    void switchAvatarColour();
    
    String avatarCursorName( int avatarIndex );

    void randomMove( const String& cursorName, int y1, int y2, int x1, int x2, int charBase, int animation );


    InputDevice& m_inputDevice;
    WindowManager& m_windowManager;
    String m_windowName;
    EntropySource& m_entropySource;
    Timer* m_timer;

    Value m_parameters;
    enum State m_state;
    bool m_completed;
    Value m_returnParameters;

    Terminal* m_terminal;
    Forms::Form* m_currentForm;

    int m_avatarIndex;
    int m_avatarColour;

    int m_avatarAnimation;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_STRATEGIES_CHOOSE_AVATAR_H
