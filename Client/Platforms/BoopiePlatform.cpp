#include "GraphicsDrivers/GraphicsDriver.h"
#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/StrToHex.h"
#include "Utils/LiteStream.h"
#include "BatteryMonitor.h"
#include "BoopiePlatform.h"
#include "BusAddresses.h"
#include "BusController.h"
#include "Collections.h"
#include "KiamaFS.h"
#include "SPIRequester.h"
#include "SPIRequests.h"
#include "String.h"
#include "Version.h"

#include <string.h>

#include <xc.h>

namespace
{
    const int powerStateQueryPeriod( 1000 ); // ms
    const int keyboardBrightnessQueryPeriod( 10000 ); // ms
    const int sensorsQueryPeriod( 10000 ); // ms
    const int waitTime( 5 ); // ms

    const int cellCapacity( 2600 ); // mAh. FIXME: Make run-time configurable.
    const double capacityFracAtMaxCharge( 0.90 ); // FIXME: Make run-time configurable.

    const int maxKeyboardBacklightCount( 8000 );
} // Anonymous namespace

namespace Agape
{

namespace Platforms
{

char* Boopie::s_heapStart( nullptr );

Boopie::Boopie( GraphicsDriver& graphicsDriver,
                BusController& bus,
                SPIRequester& spiRequester,
                Timers::Factory& timerFactory,
                KiamaFS* fs ) :
  m_graphicsDriver( graphicsDriver ),
  m_bus( bus ),
  m_spiRequester( spiRequester ),
  m_fs( fs ),
  m_powerStateQueryTimer( timerFactory.makeTimer() ),
  m_keyboardBrightnessQueryTimer( timerFactory.makeTimer() ),
  m_sensorsQueryTimer( timerFactory.makeTimer() ),
  m_blockingQueryTimer( timerFactory.makeTimer() ),
  m_readPowerStateSent( false ),
  m_userPortCurrent( 0 ),
  m_needSendKeyboardBrightnessUp( false ),
  m_needSendKeyboardBrightnessDown( false ),
  m_needSendSetKeyboardBrightness( false ),
  m_readKeyboardBrightnessSent( false ),
  m_keyboardBrightness( 0 ),
  m_needSendAlertState( false ),
  m_alertState( false ),
  m_readSensorsSent( false ),
  m_ambientSensorValue( 0 ),
  m_lidSensorValue( 0 ),
  m_charge( 0 ),
  m_voltage( 0 ),
  m_current( 0 ),
  m_chargerState( 0 ),
  m_flags( 0 )
{
}

Boopie::~Boopie()
{
    delete( m_powerStateQueryTimer );
    delete( m_keyboardBrightnessQueryTimer );
    delete( m_sensorsQueryTimer );
    delete( m_blockingQueryTimer );
}

void Boopie::performSelfTest()
{
    int chk = m_graphicsDriver.selfTest();
    LiteStream stream;
    stream << "Graphics checksum: " << ucharToHex( (unsigned char)chk );
    LOG_DEBUG( stream.str() );
}

void Boopie::doBootTasks()
{
    if( m_fs )
    {
        m_fs->purge();
        m_fs->createIndex();
    }
}

bool Boopie::error()
{
    return m_graphicsDriver.error();
}

void Boopie::currentErrors( Vector< enum ErrorType >& errors )
{
    if( m_graphicsDriver.error() ) errors.push_back( displayError );
}

long Boopie::heapUsed()
{
    char* heapEnd( new char );
    long heapUsed( (long)heapEnd - (long)s_heapStart );
    delete( heapEnd );
    return heapUsed;
}

int Boopie::userRead( int bit )
{
    unsigned char data;
    m_bus.read( BusAddresses::CSX1, (char*)&data, 1 );
    return( ( data & ( 1 << bit ) ) >> bit );
}

void Boopie::userWrite( int bit, int value )
{
    m_userPortCurrent &= ~( 1 << bit );
    m_userPortCurrent += ( value << bit );
    m_bus.write( BusAddresses::CSX1, (char*)&m_userPortCurrent, 1 );
}

void Boopie::testBus()
{
    char c( '\xff' );
    m_bus.write( BusAddresses::CSX1, &c, 1 );
}

struct Platform::PowerState Boopie::powerState()
{
    return m_powerState;
}

void Boopie::screenBrightnessUp()
{
    m_graphicsDriver.screenBrightnessUp();

    // Send event immediately.
    Event event;
    event.m_type = Platform::screenBrightnessChanged;
    dispatchEvent( event ); // Prompts UI to call getScreenBrightness() to retrieve and save to configutation store.
}

void Boopie::screenBrightnessDown()
{
    m_graphicsDriver.screenBrightnessDown();

    // Send event immediately.
    Event event;
    event.m_type = Platform::screenBrightnessChanged;
    dispatchEvent( event ); // Prompts UI to call getScreenBrightness() to retrieve and save to configutation store.
}

int Boopie::getScreenBrightness()
{
    return m_graphicsDriver.getScreenBrightness();
}

void Boopie::setScreenBrightness( int brightness )
{
    m_graphicsDriver.setScreenBrightness( brightness );

    // Send event immediately.
    Event event;
    event.m_type = Platform::screenBrightnessChanged;
    dispatchEvent( event ); // Prompts UI to call getScreenBrightness() to retrieve and save to configutation store.
}

void Boopie::keyboardBrightnessUp()
{
    m_needSendKeyboardBrightnessUp = true;
    // Send keyboardBrightnessChanged event next time we poll the current brightness.
}

void Boopie::keyboardBrightnessDown()
{
    m_needSendKeyboardBrightnessDown = true;
    // Send keyboardBrightnessChanged event next time we poll the current brightness.
}

int Boopie::getKeyboardBrightness()
{
    return m_keyboardBrightness;
}

void Boopie::setKeyboardBrightness( int brightness )
{
    m_keyboardBrightness = brightness;
    m_needSendSetKeyboardBrightness = true;
    // Send keyboardBrightnessChanged event next time we poll the current brightness.
}

void Boopie::notify( enum NotifyType type, enum NotifySource source )
{
    m_alertState = true;
    m_needSendAlertState = true;
}

void Boopie::cancelNotify( enum NotifyType type )
{
    m_alertState = false;
    m_needSendAlertState = true;
}

void Boopie::readSensors( char* data, int maxLength )
{
    if( maxLength >= 4 )
    {
        data[0] = m_ambientSensorValue;
        data[1] = m_ambientSensorValue >> 8;
        data[2] = m_lidSensorValue;
        data[3] = m_lidSensorValue >> 8;
    }
}

String Boopie::internalState()
{
    LiteStream stream;
    stream << "Charge: " << (int)m_charge
           << " Voltage: " << (int)m_voltage
           << " Current: " << (int)m_current
           << " State: ";
    switch( m_chargerState )
    {
    case 1:
        stream << "P";
        break;
    case 2:
        stream << "c";
        break;
    case 3:
        stream << "C";
        break;
    case 4:
        stream << "D";
        break;
    case 5:
        stream << "X";
        break;
    default:
        stream << "?";
        break;
    }
    stream << " Flags: ";
    if( ( m_flags & 1 ) == 1 )
    {
        stream << "V";
    }
    if( ( m_flags & 2 ) == 2 )
    {
        stream << "E";
    }
    if( ( m_flags & 4 ) == 4 )
    {
        stream << "F";
    }
    stream << " Ambient: " << m_ambientSensorValue;
    stream << " Lid: " << m_lidSensorValue;
    stream << "\r\n";
    return stream.str();
}

int Boopie::buildNumber()
{
    return g_buildNumber;
}

void Boopie::reset()
{
    // Per PIC32 FRM s. 7.3.4.
    __builtin_disable_interrupts();
    SYSKEY = 0x00; // Unlock
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    RSWRSTSET = 1; // Write to arm
    unsigned int dummy;
    dummy = RSWRST; // Read to trigger
    while(1);
}

void Boopie::run()
{
    // FIXME: In future this copypasta for async reads should be handled
    // using the promise/future pattern, rather than having a million member
    // variables in each class where it's done.
    if( !m_readPowerStateSent &&
        ( m_powerStateQueryTimer->ms() >= powerStateQueryPeriod ) &&
        !m_spiRequester.busy() )
    {
#ifdef LOG_SPI
        LOG_DEBUG( "==Reading power state==" );
#endif
        m_spiRequester.sendRequest( SPIReadPowerState,
                                    nullptr,
                                    0 );
        m_readPowerStateSent = true;
        m_powerStateQueryTimer->reset();
    }

    if( m_readPowerStateSent &&
        ( m_powerStateQueryTimer->ms() >= waitTime ) )
    {
        char response[maxSPIPayloadLength];
        int responseLength( m_spiRequester.readResponse( response ) );

        if( responseLength == 8 )
        {
#ifdef LOG_SPI
            LOG_DEBUG( "Decoding power state" );
#endif
            decodePowerState( response ); // Decodes to m_powerState.

            Event event;
            event.m_type = Platform::powerStateChanged;
            dispatchEvent( event ); // Prompts UI to call powerState() to retrieve.
        }
        else
        {
            LiteStream stream;
            stream << "BoopiePlatform: Invalid response length " << responseLength << " reading power state";
            LOG_DEBUG( stream.str() );
            hexDump( response, responseLength );
        }

#ifdef LOG_SPI
        LOG_DEBUG( "==Done reading power state==" );
#endif

        m_readPowerStateSent = false;
        m_powerStateQueryTimer->reset();
    }

    if( m_needSendKeyboardBrightnessUp &&
        !m_spiRequester.busy() )
    {
#ifdef LOG_SPI
        LOG_DEBUG( "==Sending illumination up==" );
#endif
        m_spiRequester.sendRequest( SPISetIlluminationUp,
                                    nullptr,
                                    0 );
        m_blockingQueryTimer->reset();
        while( m_blockingQueryTimer->ms() < waitTime ) {}
        char response[maxSPIPayloadLength];
        m_spiRequester.readResponse( response );
        m_needSendKeyboardBrightnessUp = false;
    }

    if( m_needSendKeyboardBrightnessDown &&
        !m_spiRequester.busy() )
    {
#ifdef LOG_SPI
        LOG_DEBUG( "==Sending illumination down==" );
#endif
        m_spiRequester.sendRequest( SPISetIlluminationDown,
                                    nullptr,
                                    0 );
        m_blockingQueryTimer->reset();
        while( m_blockingQueryTimer->ms() < waitTime ) {}
        char response[maxSPIPayloadLength];
        m_spiRequester.readResponse( response );
        m_needSendKeyboardBrightnessDown = false;
    }

    if( m_needSendSetKeyboardBrightness &&
        !m_spiRequester.busy() )
    {
#ifdef LOG_SPI
        LOG_DEBUG( "==Sending illumination value==" );
#endif
        char request[2];
        request[0] = m_keyboardBrightness & 0xFF;
        request[1] = ( m_keyboardBrightness >> 8 ) & 0xFF;
        m_spiRequester.sendRequest( SPISetIllumination,
                                    request,
                                    2 );
        m_blockingQueryTimer->reset();
        while( m_blockingQueryTimer->ms() < waitTime ) {}
        char response[maxSPIPayloadLength];
        m_spiRequester.readResponse( response );
        m_needSendSetKeyboardBrightness = false;
    }

    if( m_needSendAlertState &&
        !m_spiRequester.busy() )
    {
#ifdef LOG_SPI
        LOG_DEBUG( "==Sending alert state==" );
#endif
        char request( m_alertState ? 1 : 0 );
        m_spiRequester.sendRequest( SPISetAlert,
                                    &request,
                                    1 );
        m_blockingQueryTimer->reset();
        while( m_blockingQueryTimer->ms() < waitTime ) {}
        char response[maxSPIPayloadLength];
        m_spiRequester.readResponse( response );
        m_needSendAlertState = false;
    }

    if( !m_readKeyboardBrightnessSent &&
        ( m_keyboardBrightnessQueryTimer->ms() >= keyboardBrightnessQueryPeriod ) &&
        !m_spiRequester.busy() )
    {
#ifdef LOG_SPI
        LOG_DEBUG( "==Reading keyboard brightness==" );
#endif
        m_spiRequester.sendRequest( SPIReadIllumination,
                                    nullptr,
                                    0 );
        m_readKeyboardBrightnessSent = true;
        m_keyboardBrightnessQueryTimer->reset();
    }

    if( m_readKeyboardBrightnessSent &&
        ( m_keyboardBrightnessQueryTimer->ms() >= waitTime ) )
    {
        char response[maxSPIPayloadLength];
        int responseLength( m_spiRequester.readResponse( response ) );

        if( responseLength == 2 )
        {
#ifdef LOG_SPI
            LOG_DEBUG( "Decoding keyboard brightness" );
#endif
            m_keyboardBrightness = *( (unsigned char*)response );
            m_keyboardBrightness += ( (int)*( (unsigned char*)response + 1 ) ) << 8;

            Event event;
            event.m_type = Platform::keyboardBrightnessChanged;
            dispatchEvent( event ); // Prompts UI to call getKeyboardBrightness() to retrieve and save to configuration store.
        }

#ifdef LOG_SPI
        LOG_DEBUG( "==Done reading keyboard brightness==" );
#endif

        m_readKeyboardBrightnessSent = false;
        m_keyboardBrightnessQueryTimer->reset();
    }

    if( !m_readSensorsSent &&
        ( m_sensorsQueryTimer->ms() >= sensorsQueryPeriod ) &&
        !m_spiRequester.busy() )
    {
#ifdef LOG_SPI
        LOG_DEBUG( "==Reading sensors==" );
#endif
        m_spiRequester.sendRequest( SPIReadSensors,
                                    nullptr,
                                    0 );
        m_readSensorsSent = true;
        m_sensorsQueryTimer->reset();
    }

    if( m_readSensorsSent &&
        ( m_sensorsQueryTimer->ms() >= waitTime ) )
    {
        char response[maxSPIPayloadLength];
        int responseLength( m_spiRequester.readResponse( response ) );

        if( responseLength == 4 )
        {
#ifdef LOG_SPI
            LOG_DEBUG( "Decoding sensors" );
#endif
            unsigned short rawAmbientSensorValue( 0 );
            ::memcpy( &rawAmbientSensorValue, &response[0], 2 );
            m_ambientSensorValue = rawAmbientSensorValue;
            unsigned short rawLidSensorValue( 0 );
            ::memcpy( &rawLidSensorValue, &response[2], 2 );
            m_lidSensorValue = rawLidSensorValue;

            // TODO: Send changed event?
        }

#ifdef LOG_SPI
        LOG_DEBUG( "==Done reading sensors==" );
#endif

        m_readSensorsSent = false;
        m_sensorsQueryTimer->reset();
    }
}

void Boopie::setHeapStart()
{
    s_heapStart = new char;
}

void Boopie::decodePowerState( char* encodedState )
{
    // FIXME: Do we want to do some sort of sanity checking here?
    ::memcpy( &m_charge, encodedState, 2 );
    ::memcpy( &m_voltage, encodedState + 2, 2 );
    ::memcpy( &m_current, encodedState + 4, 2 );
    m_chargerState = encodedState[6];
    m_flags = encodedState[7];

#ifdef LOG_SPI
    LOG_DEBUG( internalState() );
#endif

    switch( m_chargerState )
    {
    case 1: // Pending
        m_powerState.m_chargerState = pending;
        m_powerState.m_batteryPct = 0.0;
        break;
    case 2: // Charging
        m_powerState.m_chargerState = charging;
        m_powerState.m_batteryPct = batteryPctFromCharge( m_charge );
        break;
    case 3: // Charged
        m_powerState.m_chargerState = charged;
        m_powerState.m_batteryPct = batteryPctFromCharge( m_charge );
        break;
    case 4: // Discharging
        m_powerState.m_chargerState = discharging;
        m_powerState.m_batteryPct = batteryPctFromCharge( m_charge );
        break;
    case 5: // BattError
    case 0: // Unknown
    default:
        m_powerState.m_chargerState = battError;
        m_powerState.m_batteryPct = 0.0;
        break;
    }

    m_powerState.m_extPower = ( ( m_flags & 2 ) == 2 );
}

double Boopie::batteryPctFromCharge( short charge )
{
    double frac( (double)charge / (double)cellCapacity );

    if( frac < 0.0 )
    {
        frac = 0.0;
    }
    else if( frac > capacityFracAtMaxCharge )
    {
        frac = capacityFracAtMaxCharge; // Don't show greater than fully charged.
    }

    return( frac * 100 );
}

} // namespace Platforms

} // namespace Agape
