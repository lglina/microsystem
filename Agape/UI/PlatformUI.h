#ifndef AGAPE_UI_PLATFORMUI_H
#define AGAPE_UI_PLATFORMUI_H

#include "Platforms/Platform.h"
#include "Runnable.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class InputDevice;
class String;
class Terminal;
class Timer;
class WindowManager;

namespace UI
{

class TabBar;

class PlatformUI : public Platform::EventListener, public Runnable
{
public:
    PlatformUI( Agape::Platform& platform,
                WindowManager& windowManager,
                const String& modalWindowName,
                TabBar& tabBar,
                InputDevice& inputDevice,
                Timers::Factory& timerFactory );
    ~PlatformUI();

    virtual void receiveEvent( const Platform::Event& event );

    virtual void run();

    void showAssetModal( const String& assetName );

private:
    void updatePowerState( struct Platform::PowerState& powerState );
    void updateHeapState();

    void showGraphicsWarning();

    Agape::Platform& m_platform;
    WindowManager& m_windowManager;
    TabBar& m_tabBar;
    InputDevice& m_inputDevice;

    Terminal* m_modalTerminal;

    Timer* m_updateTimer;
    Timer* m_modalTimer;

    bool m_graphicsWarningShown;

    long m_peakHeapUsed;
};

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_PLATFORMUI_H
