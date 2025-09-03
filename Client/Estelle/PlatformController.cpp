#include "InputDevices/BetaKeyboard.h"
#include "InputDevices/InputDevice.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Collections.h"
#include "PlatformController.h"
#include "PowerControllable.h"
#include "SPIRequests.h"

#include <string.h>

#include <xc.h>

namespace
{
    const int resetTime( 100 ); // ms
} // Anonymous namespace

namespace Agape
{

PlatformController::PlatformController( InputDevices::BetaKeyboard& inputDevice,
                                        Timers::Factory& timerFactory ) :
    m_inputDevice( inputDevice ),
    m_resetTimer( timerFactory.makeTimer() ),
    m_sampleTimer( timerFactory.makeTimer() ),
    m_powerOn( false ),
    m_holdingReset( false )
{
}

PlatformController::~PlatformController()
{
    delete( m_resetTimer );
    delete( m_sampleTimer );
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
        return 0;
    }
    else if( requestType == SPIReadSensors )
    {
        // FIXME: Rather than blocking here, we should have an object doing
        // background ADC sampling continuously on all used channels and share
        // this with BatteryMonitor.

        // Assumption: Sensors are already powered.
        AD1CON3bits.ADCS = 8; // No divider needed for TAD >= 200ns for 10-bit mode, per DS param AD50A.

        // Save off channel previously selected by BatteryMonitor and stop
        // sampling. Do our thing, then re-select and start sampling that
        // channel again. This will probably screw up BatteryMonitor!
        int prevChannel = AD1CHSbits.CH0SA;
        AD1CON1bits.SAMP = 0;

        AD1CHSbits.CH0SA = 11; // AN11 = ambient light sensor.
        AD1CON1bits.ON = 1;
        AD1CON1bits.SAMP = 1; // Start sampling
        m_sampleTimer->reset();
        while( m_sampleTimer->ms() < 10 ) {} // ??
        AD1CON1bits.DONE = 0;
        AD1CON1bits.SAMP = 0; // Start conversion
        m_sampleTimer->reset();
        while( ( AD1CON1bits.DONE == 0 ) && ( m_sampleTimer->ms() < 10 ) ) {} // Wait for conversion to complete
        int sample = ADC1BUF0;
        ::memcpy( &response[0], &sample, 2 );

        AD1CHSbits.CH0SA = 4; // AN4 = lid magnetic sensor.
        AD1CON1bits.ON = 1;
        AD1CON1bits.SAMP = 1; // Start sampling
        m_sampleTimer->reset();
        while( m_sampleTimer->ms() < 10 ) {} // ??
        AD1CON1bits.DONE = 0;
        AD1CON1bits.SAMP = 0; // Start conversion
        m_sampleTimer->reset();
        while( ( AD1CON1bits.DONE == 0 ) && ( m_sampleTimer->ms() < 10 ) ) {} // Wait for conversion to complete
        sample = ADC1BUF0;
        ::memcpy( &response[2], &sample, 2 );

        AD1CHSbits.CH0SA = prevChannel;
        AD1CON1bits.SAMP = 1;

        return 4;
    }

    return 0;
}

} // namespace Agape
