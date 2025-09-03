#ifndef AGAPE_UI_STRATEGIES_MESSAGE_H
#define AGAPE_UI_STRATEGIES_MESSAGE_H

#include "UI/Strategy.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{
class Factory;
} // namespace AssetLoaders

namespace Timers
{
class Factory;
} // namespace Timers

namespace World
{
class Coordinates;
} // namespace World

using namespace World;

class ConfigurationStore;
class InputDevice;
class Terminal;
class Timer;
class Value;
class WindowManager;

namespace UI
{

namespace Strategies
{

class Message : public Strategy
{
public:
    Message( WindowManager& windowManager,
             const String& windowName,
             InputDevice& inputDevice,
             Timers::Factory& timerFactory,
             AssetLoaders::Factory& assetLoaderFactory,
             Coordinates& coordinates,
             ConfigurationStore& configurationStore );
    ~Message();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    void drawCountdown();

    WindowManager& m_windowManager;
    String m_windowName;
    InputDevice& m_inputDevice;
    Timer* m_timer;
    AssetLoaders::Factory& m_assetLoaderFactory;
    Coordinates& m_coordinates;
    ConfigurationStore& m_configurationStore;

    bool m_completed;

    int m_lastSecond;

    Terminal* m_terminal;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_STRATEGIES_MESSAGE_H
