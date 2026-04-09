#ifndef AGAPE_PLATFORM_CONTROLLER_H
#define AGAPE_PLATFORM_CONTROLLER_H

#include "Collections.h"
#include "PowerControllable.h"
#include "Runnable.h"
#include "SPIResponder.h"

namespace Agape
{

namespace InputDevices
{
class BetaKeyboard;
} // namespace InputDevices

namespace Timers
{
class Factory;
} // namespace Timers

class PICADC;
class Timer;

class PlatformController : public SPIResponseSource
{
public:
    PlatformController( InputDevices::BetaKeyboard& inputDevice,
                        PICADC& picADC,
                        Timers::Factory& timerFactory );

    ~PlatformController();

    virtual void run();

    void registerPowerControllable( PowerControllable* powerControllable );

    void setPowerState( enum PowerControllable::PowerState powerState );

    virtual int spiResponse( char requestType,
                             char* request,
                             int requestLength,
                             char* response,
                             int maxResponseLength );

private:
    void doSleep();

    InputDevices::BetaKeyboard& m_inputDevice;
    PICADC& m_picADC;

    Timer* m_resetTimer;
    Timer* m_sleepTimer;

    bool m_powerOn;
    bool m_holdingReset;

    Vector< PowerControllable* > m_powerControllables;
};

} // namespace Agape

#endif // AGAPE_PLATFORM_CONTROLLER_H
