#include "InputDevices/BetaKeyboard.h"
#include "InputDevices/InputDevice.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Collections.h"
#include "PICADC.h"
#include "PlatformController.h"
#include "PowerControllable.h"
#include "SPIRequests.h"

#include <string.h>

#include <xc.h>

namespace
{
    const int resetTime( 100 ); // ms
    const int sleepTime( 10000 ); // ms

    const int channelAmbientLight = 11;
    const int channelMagneticSensor = 4;
} // Anonymous namespace

namespace Agape
{

PlatformController::PlatformController( InputDevices::BetaKeyboard& inputDevice,
                                        PICADC& picADC,
                                        Timers::Factory& timerFactory ) :
    m_inputDevice( inputDevice ),
    m_picADC( picADC ),
    m_resetTimer( timerFactory.makeTimer() ),
    m_sleepTimer( timerFactory.makeTimer() ),
    m_powerOn( false ),
    m_holdingReset( false )
{
    m_picADC.addChannel( channelAmbientLight );
    m_picADC.addChannel( channelMagneticSensor );
}

PlatformController::~PlatformController()
{
    delete( m_resetTimer );
    delete( m_sleepTimer );
}

void PlatformController::run()
{
    if( m_inputDevice.escHeld() )
    {
        if( m_powerOn )
        {
            LATBCLR = _LATB_LATB4_MASK; // Assert reset.
            LATBSET = _LATB_LATB5_MASK; // Power off.
            m_powerOn = false;
            setPowerState( PowerControllable::off );
            m_sleepTimer->reset();
        }
        else
        {
            LATBCLR = _LATB_LATB4_MASK | _LATB_LATB5_MASK; // Power on, assert reset.
            m_powerOn = true;
            m_holdingReset = true;
            m_resetTimer->reset();
            setPowerState( PowerControllable::on );
        }
    }

    if( m_holdingReset && ( m_resetTimer->ms() >= resetTime ) )
    {
        LATBSET = _LATB_LATB4_MASK; // Clear reset.
        m_holdingReset = false;
    }

    if( !m_powerOn && ( m_sleepTimer->ms() >= sleepTime ) )
    {
        doSleep();
    }
}

void PlatformController::registerPowerControllable( PowerControllable* powerControllable )
{
    m_powerControllables.push_back( powerControllable );
}

void PlatformController::setPowerState( enum PowerControllable::PowerState powerState )
{
    Vector< PowerControllable* >::iterator it( m_powerControllables.begin() );
    for( ; it != m_powerControllables.end(); ++it )
    {
        ( *it )->setPowerState( powerState );
    }
}

int PlatformController::spiResponse( char requestType,
                                     char* request,
                                     int requestLength,
                                     char* response,
                                     int maxResponseLength )
{
    int responseLength( 0 );

    if( requestType == SPISetAlert )
    {
        if( requestLength == 1 )
        {
            if( request[0] == 0 )
            {
                LATBSET = _LATB_LATB10_MASK;
            }
            else
            {
                LATBCLR = _LATB_LATB10_MASK;
            }
        }
        // responseLength = 0;
    }
    else if( requestType == SPIReadSensors )
    {
        int ambientValue( 0 );
        int magneticValue( 0 );
        if( m_picADC.getValue( channelAmbientLight, ambientValue ) &&
            m_picADC.getValue( channelMagneticSensor, magneticValue ) )
        {
            ::memcpy( &response[0], &ambientValue, 2 );
            ::memcpy( &response[2], &magneticValue, 2 );

            responseLength = 4;
        }
        // responseLength = 0;
    }

    return responseLength;
}

void PlatformController::doSleep()
{
    m_inputDevice.prepareEscSleep(); // Set up change notification interrupt for wake.

    __builtin_disable_interrupts();
    SYSKEY = 0x00000000; // Force lock
    SYSKEY = 0xAA996655; // Unlock
    SYSKEY = 0x556699AA;
    PWRCONbits.RETEN = 1; // Enable retention sleep (RETVR config bit must be programmed ON (0)).
    PWRCONbits.VREGS = 1; // Must be set 1 for retention sleep.
    OSCCONbits.SLPEN = 1; // Arm sleep mode.
    SYSKEY = 0x00000000; // Lock

    PORTBCLR = _PORTB_RB7_MASK; // Disable sensors.

    __builtin_enable_interrupts();

    asm( "wait" );          // Enter sleep mode.

    m_inputDevice.afterEscSleep(); // Disable change notification interrupt.

    m_sleepTimer->reset();

    PORTBSET = _PORTB_RB7_MASK; // Enable sensors.
}

} // namespace Agape
