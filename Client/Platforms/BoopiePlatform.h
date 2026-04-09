#ifndef AGAPE_PLATFORMS_BOOPIE_H
#define AGAPE_PLATFORMS_BOOPIE_H

#include "Platforms/Platform.h"
#include "Collections.h"
#include "String.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class BusController;
class GraphicsDriver;
class KiamaFS;
class SPIRequester;
class Timer;

namespace Platforms
{

class Boopie : public Platform
{
public:
    Boopie( GraphicsDriver& graphicsDriver,
            BusController& bus,
            SPIRequester& spiRequester,
            Timers::Factory& timerFactory,
            KiamaFS* fs = nullptr );
    ~Boopie();

    virtual void performSelfTest();

    virtual void doBootTasks();

    virtual bool error();
    virtual void currentErrors( Vector< enum ErrorType >& errors );

    virtual long heapUsed();

    virtual int userRead( int bit );
    virtual void userWrite( int bit, int value );
    virtual void testBus();

    virtual struct PowerState powerState();

    virtual void screenBrightnessUp();
    virtual void screenBrightnessDown();
    virtual int getScreenBrightness();
    virtual void setScreenBrightness( int brightness );

    virtual void keyboardBrightnessUp();
    virtual void keyboardBrightnessDown();
    virtual int getKeyboardBrightness();
    virtual void setKeyboardBrightness( int brightness );

    virtual void notify( enum NotifyType type, enum NotifySource source );
    virtual void cancelNotify( enum NotifyType type );

    virtual void readSensors( char* data, int maxLength );

    virtual String internalState();

    virtual int buildNumber();

    virtual void reset();

    virtual void run();

    static void setHeapStart();

private:
    void decodePowerState( char* encodedState );
    double batteryPctFromCharge( short charge );

    GraphicsDriver& m_graphicsDriver;
    BusController& m_bus;
    SPIRequester& m_spiRequester;
    KiamaFS* m_fs;
    
    Timer* m_powerStateQueryTimer;
    Timer* m_keyboardBrightnessQueryTimer;
    Timer* m_sensorsQueryTimer;
    Timer* m_blockingQueryTimer;

    bool m_readPowerStateSent;
    struct PowerState m_powerState;

    unsigned char m_userPortCurrent;

    static char* s_heapStart;

    bool m_needSendKeyboardBrightnessUp;
    bool m_needSendKeyboardBrightnessDown;
    bool m_needSendSetKeyboardBrightness;
    bool m_readKeyboardBrightnessSent;
    int m_keyboardBrightness;

    bool m_needSendAlertState;
    bool m_alertState;

    bool m_readSensorsSent;
    int m_ambientSensorValue;
    int m_lidSensorValue;

    // Last raw state from Estelle
    short m_charge;
    short m_voltage;
    short m_current;
    char m_chargerState;
    char m_flags;
};

} // namespace Platforms

} // namespace Agape

#endif // AGAPE_PLATFORMS_BOOPIE_H
