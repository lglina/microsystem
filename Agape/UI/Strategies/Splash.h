#ifndef AGAPE_UI_STRATEGIES_SPLASH_H
#define AGAPE_UI_STRATEGIES_SPLASH_H

#include "UI/Strategy.h"
#include "String.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class InputDevice;
class Platform;
class Timer;
class Value;
class WindowManager;

namespace UI
{

namespace Strategies
{

class Splash : public Strategy
{
public:
    Splash( WindowManager& windowManager,
            InputDevice& inputDevice,
            Timers::Factory& timerFactory,
            Platform& platform );
    ~Splash();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    void checkForKey();

    WindowManager& m_windowManager;
    InputDevice& m_inputDevice;
    Timer* m_timer;
    Platform& m_platform;

    int m_assetNumber;
    int m_splashDelay;

    String m_nextStrategy;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_STRATEGIES_SPLASH_H
