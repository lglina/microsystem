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

    virtual void brightnessUp();
    virtual void brightnessDown();

    virtual void keyboardBrightnessUp();
    virtual void keyboardBrightnessDown();

    virtual void notify( enum NotifyType type, enum NotifySource source );
    virtual void cancelNotify( enum NotifyType type );

    virtual void readSensors( char* data, int maxLength );

    virtual String internalState();

    virtual void run();

    static void setHeapStart();

private:
    void decodePowerState( char* encodedState );
    double batteryPctFromCharge( short charge );

    GraphicsDriver& m_graphicsDriver;
    BusController& m_bus;
    SPIRequester& m_spiRequester;
    KiamaFS* m_fs;
    Timer* m_estelleQueryTimer;
    Timer* m_blockingQueryTimer;

    bool m_estelleRequestSent;
    struct PowerState m_powerState;

    unsigned char m_userPortCurrent;

    static char* s_heapStart;

    bool m_needSendKeyboardBacklight;
    int m_keyboardBacklightCount;

    bool m_needSendAlertState;
    bool m_alertState;

    bool m_needReadSensors;
    int m_ambientSensorValue;
    int m_lidSensorValue;

    char m_lastRawState;

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
