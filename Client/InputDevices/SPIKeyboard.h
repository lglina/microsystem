#ifndef AGAPE_INPUT_DEVICES_SPI_KEYBOARD_H
#define AGAPE_INPUT_DEVICES_SPI_KEYBOARD_H

#include "InputDevices/InputDevice.h"
#include "Utils/RingBuffer.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class SPIRequester;
class Timer;

namespace InputDevices
{

class SPIKeyboard : public InputDevice
{
public:
    SPIKeyboard( SPIRequester& spiRequester,
                 Timers::Factory& timerFactory );
    ~SPIKeyboard();

    virtual bool eof();
    virtual char peek();
    virtual char get();

    virtual void run();

private:
    SPIRequester& m_spiRequester;

    Timer* m_timer;

    RingBuffer< char > m_buffer;

    bool m_requestSent;
};

} // namespace InputDevices

} // namespace Agape

#endif // AGAPE_INPUT_DEVICES_SPI_KEYBOARD_H
