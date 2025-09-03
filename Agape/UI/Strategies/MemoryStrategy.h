#ifndef AGAPE_UI_STRATEGIES_MEMORY_H
#define AGAPE_UI_STRATEGIES_MEMORY_H

#include "UI/Strategy.h"
#include "String.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class InputDevice;
class Memory;
class ReadableWritable;
class Terminal;
class Timer;
class Value;
class WindowManager;

namespace UI
{

namespace Strategies
{

class Memory : public Strategy
{
public:
    Memory( WindowManager& windowManager,
            const String& windowName,
            InputDevice& inputDevice,
            Agape::Memory& memory,
            ReadableWritable& serialPort,
            Timers::Factory& timerFactory );
    ~Memory();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    InputDevice& m_inputDevice;
    Agape::Memory& m_memory;
    ReadableWritable& m_serialPort;

    Terminal* m_terminal;
    Timer* m_delayTimer;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_STRATEGIES_MEMORY_H
