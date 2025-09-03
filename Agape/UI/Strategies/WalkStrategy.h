#ifndef AGAPE_UI_STRATEGIES_WALK_H
#define AGAPE_UI_STRATEGIES_WALK_H

#include "UI/Strategy.h"
#include "String.h"

using namespace Agape::World;

namespace Agape
{

namespace Audio
{
class MIDIPlayer;
} // namespace Audio

using namespace Audio;

namespace Timers
{
class Factory;
} // namespace Timers

namespace World
{
class Compositor;
class Coordinates;
class Metadata;
class User;
} // namespace World

class Chat;
class Clock;
class EntropySource;
class InputDevice;
class Line;
class Platform;
class Timer;
class Value;

namespace UI
{

class Dialogue;
class Hotkeys;
class Navigation;
class NotificationsUI;
class PlatformUI;
class Presence;
class VRTime;

namespace Strategies
{

class Walk : public Strategy
{
public:
    Walk( InputDevice& inputDevice,
          Compositor& compositor,
          Chat& chat,
          Metadata& worldMetadata,
          User& worldUser,
          Coordinates& coordinates,
          Hotkeys& hotkeys,
          Navigation& navigation,
          Presence& presence,
          VRTime& vrTime,
          Platform& platform,
          PlatformUI& platformUI,
          NotificationsUI& notificationsUI,
          Timers::Factory& timerFactory,
          Strategy& onboardingStrategy,
          EntropySource& entropySource,
          Clock& clock,
          Line& line,
          Dialogue& dialogue,
          MIDIPlayer& midiPlayer );
    ~Walk();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        normal,
        connLostQuery,
        connLostReconnect,
        offline
    };

    void doWalk();

    void drawHotkeys();

    void setHolidays();

    void createWeather();
    void updateWeather();

    InputDevice& m_inputDevice;
    
    Compositor& m_compositor;
    Chat& m_chat;

    Metadata& m_worldMetadata;
    User& m_worldUser;

    Coordinates& m_coordinates;

    Hotkeys& m_hotkeys;
    Navigation& m_navigation;
    Presence& m_presence;
    VRTime& m_vrTime;
    Platform& m_platform;
    PlatformUI& m_platformUI;
    NotificationsUI& m_notificationsUI;
    Strategy& m_onboardingStrategy;
    EntropySource& m_entropySource;
    Clock& m_clock;
    Line& m_line;
    Dialogue& m_dialogue;
    MIDIPlayer& m_midiPlayer;

    enum State m_state;

    bool m_completed;
    bool m_calling;
    Value m_callingParameters;
    String m_nextStrategy;
    Value m_returnParameters;

    Timer* m_presenceUpdateTimer;
    Timer* m_notificationsUpdateTimer;
    Timer* m_weatherTimer;
    Timer* m_inactivityTimer;
    Timer* m_reconnectTimer;

    bool m_holidays;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_STRATEGIES_WALK_H
