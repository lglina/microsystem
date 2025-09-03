#ifndef AGAPE_UI_STRATEGIES_TEST_H
#define AGAPE_UI_STRATEGIES_TEST_H

#include "UI/Strategy.h"
#include "String.h"

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

class EntropySource;
class InputDevice;
class Memory;
class Platform;
class Terminal;
class Timer;
class Value;
class WindowManager;

namespace UI
{

namespace Strategies
{

class Test : public Strategy
{
public:
    Test( WindowManager& windowManager,
          const String& windowName,
          InputDevice& inputDevice,
          Platform& platform,
          Agape::Memory& memory,
          MIDIPlayer& midiPlayer,
          EntropySource& entropySource,
          Timers::Factory& timerFactory );

    ~Test();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        TestScreen,
        TestPowerSupply,
        TestKeyboard,
        TestFlash,
        TestModem,
        TestSound,
        TestRNG
    };

    void setState();

    InputDevice& m_inputDevice;
    Platform& m_platform;
    Agape::Memory& m_memory;
    MIDIPlayer& m_midiPlayer;
    EntropySource& m_entropySource;

    Terminal* m_terminal;

    Timer* m_timer;

    enum State m_state;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_STRATEGIES_TEST_H
