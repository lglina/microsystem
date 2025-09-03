#ifndef AGAPE_PLATFORM_H
#define AGAPE_PLATFORM_H

#include "Collections.h"
#include "Runnable.h"
#include "String.h"

namespace Agape
{

class Platform
{
public:
    enum EventType
    {
        hotPlugAttach,
        hotPlugDetach,
        powerStateChanged
    };

    enum ErrorType
    {
        displayError,
        copyError
    };

    enum NotifyType
    {
        newChat,
        newTelegram,
        connectionError
    };

    enum NotifySource
    {
        server,
        client
    };

    enum ChargerState
    {
        unknown,
        pending,
        charging,
        charged,
        discharging,
        battError
    };

    struct PowerState
    {
        PowerState() :
          m_chargerState( unknown ),
          m_extPower( false ),
          m_batteryPct( 0.0 ),
          m_runtimeHours( 0 ),
          m_runtimeMins( 0 )
        {
        }

        enum ChargerState m_chargerState;
        bool m_extPower;
        double m_batteryPct;
        int m_runtimeHours;
        int m_runtimeMins;
    };

    struct Event
    {
        enum EventType m_type;
        String m_data;
    };

    class EventListener
    {
    public:
        virtual void receiveEvent( const Event& event ) = 0;
    };

    virtual ~Platform() {};

    void registerListener( EventListener* listener );

    virtual void performSelfTest() = 0;

    virtual void doBootTasks() = 0;

    virtual bool error() = 0;
    virtual void currentErrors( Vector< enum ErrorType >& errors ) = 0;

    virtual long heapUsed() = 0;

    virtual int userRead( int bit ) { return 0; };
    virtual void userWrite( int bit, int value ) {};
    virtual void testBus() {};

    virtual struct PowerState powerState() = 0;

    virtual void brightnessUp() {};
    virtual void brightnessDown() {};

    virtual void keyboardBrightnessUp() {};
    virtual void keyboardBrightnessDown() {};

    virtual void notify( enum NotifyType type, enum NotifySource source ) {};
    virtual void cancelNotify( enum NotifyType type ) {};

    virtual void readSensors( char* data, int maxLength ) {};

    virtual String internalState() { return String(); };

    virtual void run() = 0;

protected:
    void dispatchEvent( const Event& event );

    Vector< EventListener* > m_eventListeners;
};

} // namespace Agape

#endif // AGAPE_PLATFORM_H
