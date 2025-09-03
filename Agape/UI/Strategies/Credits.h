#ifndef AGAPE_STRATEGIES_CREDITS_H
#define AGAPE_STRATEGIES_CREDITS_H

#include "UI/Strategy.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace Assets
{
class ANSIFile;
} // namespace Assets

namespace Timers
{
class Factory;
} // namespace Timers

class AssetLoader;
class InputDevice;
class Terminal;
class Timer;
class WindowManager;

namespace UI
{

namespace Strategies
{

class Credits : public Strategy
{
public:
    Credits( InputDevice& inputDevice,
             WindowManager& windowManager,
             const String& windowName,
             Timers::Factory& timerFactory );
    ~Credits();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    InputDevice& m_inputDevice;
    WindowManager& m_windowManager;
    String m_windowName;

    Timer* m_timer;

    bool m_completed;

    Terminal* m_terminal;

    AssetLoader* m_currentAssetLoader;
    Assets::ANSIFile* m_currentAsset;
    int m_currentLine;
    int m_assetOffset;
    int m_screenNumber;
    bool m_next;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_CREDITS_H
