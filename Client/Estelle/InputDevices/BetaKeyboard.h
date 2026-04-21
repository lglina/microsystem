#ifndef AGAPE_INPUT_DEVICES_BETA_KEYBOARD_H
#define AGAPE_INPUT_DEVICES_BETA_KEYBOARD_H

#include "InputDevices/InputDevice.h"
#include "Utils/RingBuffer.h"
#include "InterruptHandler.h"

namespace Agape
{

namespace Timers
{
class Factory;
}

class Timer;

namespace InputDevices
{

class BetaKeyboard : public InputDevice, public InterruptHandler
{
public:
    BetaKeyboard( Timers::Factory& timerFactory );
    ~BetaKeyboard();

    virtual bool eof() const;
    virtual char peek();
    virtual char get();

    virtual void run();

    bool escHeld();

    bool canEscSleep();
    void prepareEscSleep();
    void afterEscSleep();

    virtual void handleInterrupt( enum InterruptDispatcher::InterruptVector vector );

private:
    int m_curRow;
    int m_curCol;
    int m_activeRow;
    int m_activeCol;
    Timer* m_shiftTimer;
    Timer* m_ctrlTimer;
    Timer* m_escTimer;
    Timer* m_keyTimer;
    bool m_triggered;
    int m_lastRepeat;
    bool m_shiftHeld;
    bool m_ctrlHeld;
    bool m_escPressed;
    bool m_escHeld;
    RingBuffer< char > m_buffer;
};

} // namespace InputDevices

} // namespace Agape

#endif // AGAPE_INPUT_DEVICES_BETA_KEYBOARD_H
