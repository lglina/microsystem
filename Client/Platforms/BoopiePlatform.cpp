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

#include <string.h>

namespace
{
    const int estelleQueryPeriod( 1000 ); // ms
    const int waitTime( 5 ); // ms
    const int sensorsWaitTime( 100 ); // ms

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
  m_estelleQueryTimer( timerFactory.makeTimer() ),
  m_blockingQueryTimer( timerFactory.makeTimer() ),
  m_estelleRequestSent( false ),
  m_userPortCurrent( 0 ),
  m_needSendKeyboardBacklight( false ),
  m_keyboardBacklightCount( 2000 ),
  m_needSendAlertState( false ),
  m_alertState( false ),
  m_needReadSensors( false ),
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
    delete( m_estelleQueryTimer );
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

void Boopie::brightnessUp()
{
    m_graphicsDriver.brightnessUp();
}

void Boopie::brightnessDown()
{
    m_graphicsDriver.brightnessDown();
}

void Boopie::keyboardBrightnessUp()
{
    // FIXME: Broken.
    /*
    m_keyboardBacklightCount += 1000;
    if( m_keyboardBacklightCount > maxKeyboardBacklightCount )
    {
        m_keyboardBacklightCount = maxKeyboardBacklightCount;
    }
    m_needSendKeyboardBacklight = true;
    */
}

void Boopie::keyboardBrightnessDown()
{
    // FIXME: Broken.
    /*
    m_keyboardBacklightCount -= 1000;
    if( m_keyboardBacklightCount < 0 )
    {
        m_keyboardBacklightCount = 0;
    }
    m_needSendKeyboardBacklight = true;
    */
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
    // FIXME: This returns the previously read data and requests a new read,
    // which is currently blocking on Estelle. We need to make that non-blocking
    // on the Estelle end, and non-blocking here in run(), then set up a timer
    // to read it semi-regularly in the background.
    if( maxLength >= 4 )
    {
        data[0] = m_ambientSensorValue;
        data[1] = m_ambientSensorValue >> 8;
        data[2] = m_lidSensorValue;
        data[3] = m_lidSensorValue >> 8;
        m_needReadSensors = true;
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

void Boopie::run()
{
    if( !m_estelleRequestSent &&
        ( m_estelleQueryTimer->ms() >= estelleQueryPeriod ) &&
        !m_spiRequester.busy() )
    {
#ifdef LOG_SPI
        LOG_DEBUG( "==Reading power state==" );
#endif
        m_spiRequester.sendRequest( SPIReadPowerState,
                                    nullptr,
                                    0 );
        m_estelleRequestSent = true;
        m_estelleQueryTimer->reset();
    }

    if( m_estelleRequestSent &&
        ( m_estelleQueryTimer->ms() >= waitTime ) )
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

        m_estelleRequestSent = false;
        m_estelleQueryTimer->reset();
    }

    if( m_needSendKeyboardBacklight &&
        !m_spiRequester.busy() )
    {
        char request[2];
        request[0] = m_keyboardBacklightCount & 0xFF;
        request[1] = ( m_keyboardBacklightCount >> 8 ) & 0xFF;
        m_spiRequester.sendRequest( SPIReadPowerState,
                                    request,
                                    2 );
        m_blockingQueryTimer->reset();
        while( m_blockingQueryTimer->ms() < waitTime ) {}
        char response[maxSPIPayloadLength];
        m_spiRequester.readResponse( response );
        m_needSendKeyboardBacklight = false;
    }

    if( m_needSendAlertState &&
        !m_spiRequester.busy() )
    {
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

    if( m_needReadSensors &&
        !m_spiRequester.busy() )
    {
        m_spiRequester.sendRequest( SPIReadSensors,
                                    nullptr,
                                    0 );
        m_blockingQueryTimer->reset();
        while( m_blockingQueryTimer->ms() < sensorsWaitTime ) {}
        char response[maxSPIPayloadLength];
        int responseLength( m_spiRequester.readResponse( response ) );
        if( responseLength == 4 )
        {
            unsigned short rawAmbientSensorValue( 0 );
            ::memcpy( &rawAmbientSensorValue, &response[0], 2 );
            m_ambientSensorValue = rawAmbientSensorValue;
            unsigned short rawLidSensorValue( 0 );
            ::memcpy( &rawLidSensorValue, &response[2], 2 );
            m_lidSensorValue = rawLidSensorValue;
        }
        m_needReadSensors = false;
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
