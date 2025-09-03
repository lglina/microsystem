#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/LiteStream.h"
#include "BatteryMonitor.h"
#include "SPIRequests.h"

#include <math.h>
#include <string.h>

#include <xc.h>

/*
Pending (initial state):
    * Not fast charging and charge/discharge current < 50mA, after 5s, estimate
      SoC from OCV. If extpwr -> Charging, if !extpwr -> Discharging.

Charging (integrate current, SoC increasing):
    * Extpwr disconnected -> Discharging
    * Not fast charging and voltage >= 4.05V, set to
      fully charged capacity -> Charged

Charged (SoC not changed):
    * Extpwr disconnected -> Discharging
    * Fast charging -> Charging

Discharging (integrate current, SoC decreasing):
    * Extpwr connected -> Charging
*/

namespace
{
    const int numSamples( 50 ); // Must match array dimensions in header.
    const int sampleInterval( 50 ); // ms
    const int samplesPerHour( 60 * 60 * ( 1000 / ( sampleInterval * 2 ) ) );

    const int validTimeout( 10000 ); // ms max. invalid data before returning to pending state.

    // +/- mA within which we consider the voltage reading "open circuit", and
    // therefore we can roughly estimate state of charge.
    const int ocCurrent( 50 );

    // ms. Time to wait after startup before reading OCV to estimate SOC
    // (current must be below ocCurrent and not fast charging)
    const int estimationSettlingDelay( 5000 );

    const int cellCapacity( 2600 ); // mAh. FIXME: Make run-time configurable.
    const int maxChargeVoltage( 4000 ); // V. At this threshold and with fast charge off, consider fully charged.
    const double capacityFracAtMaxCharge( 0.90 ); // FIXME: Make run-time configurable.
    const double capacityAtMaxCharge( cellCapacity * capacityFracAtMaxCharge );

    const char dischargeCurve[25] = { // Percent cell capacity from OCV, in 0.05V steps from 3.0V.
        0,   0,   0,   0,   0,
        0,   0,   0,   0,   7,
        11,  19,  37,  49,  55,
        61,  67,  72,  77,  82,
        87,  91,  95,  99, 100
    };

    // mV. Max voltage variation across sample period to consider "stable"
    // voltage. When battery is missing and charger is probing for battery,
    // charger will create periodic large voltage swings which will be above
    // this, thus we know to ignore those readings.
    const int maxVoltageDelta( 500 );
} // Anonymous namespace

namespace Agape
{

BatteryMonitor::BatteryMonitor( Timers::Factory& timerFactory ) :
  m_sampleTimer( timerFactory.makeTimer() ),
  m_pendingTimer( timerFactory.makeTimer() ),
  m_validTimer( timerFactory.makeTimer() ),
  m_state( pending ),
  m_charge( 0 ),
  m_voltageSampleIdx( 0 ),
  m_currentSampleIdx( 0 ),
  m_allSampled( false ),
  m_input( voltage ),
  m_debugCount( 0 )
{
    ::memset( m_rawVoltageSamples, 0, sizeof( int ) * numSamples );
    ::memset( m_rawCurrentSamples, 0, sizeof( int ) * numSamples );

    AD1CON3bits.ADCS = 8; // No divider needed for TAD >= 200ns for 10-bit mode, per DS param AD50A.
    AD1CHSbits.CH0SA = 9; // AN9 = battery voltage.
    AD1CON1bits.ON = 1;
    AD1CON1bits.SAMP = 1; // Start sampling

    // FIXME: We could set 12 bit resolution here, but need to be mindful of
    // the silicon errata applicable to 12-bit mode.
}

BatteryMonitor::~BatteryMonitor()
{
    delete( m_sampleTimer );
    delete( m_pendingTimer );
    delete( m_validTimer );
}

int BatteryMonitor::spiResponse( char requestType,
                                 char* request,
                                 int requestLength,
                                 char* response,
                                 int maxResponseLength )
{
    if( requestType == SPIReadPowerState )
    {
        short charge( ::lround( m_charge ) );
        short voltage( averageVoltage() );
        short current( averageCurrent() );

        ::memcpy( response, &charge, 2 );
        ::memcpy( response + 2, &voltage, 2 );
        ::memcpy( response + 4, &current, 2 );

        switch( m_state )
        {
        case pending:
            response[6] = 1;
            break;
        case charging:
            response[6] = 2;
            break;
        case charged:
            response[6] = 3;
            break;
        case discharging:
            response[6] = 4;
            break;
        case battError:
            response[6] = 5;
            break;
        case unknown:
        default:
            response[6] = 0;
            break;
        }

        response[7] = 0;
        response[7] |= dataValid() ? 1 : 0;
        response[7] |= extPower() ? 2 : 0;
        response[7] |= fastCharging() ? 4 : 0;

        return 8;
    }

    return 0;
}

void BatteryMonitor::run()
{
    if( m_sampleTimer->ms() >= sampleInterval )
    {
        m_sampleTimer->reset();

        doSample();

        if( m_allSampled )
        {
            doMonitor();
            m_allSampled = false;
        }
    }
}

void BatteryMonitor::doSample()
{
    AD1CON1bits.DONE = 0;
    AD1CON1bits.SAMP = 0; // Start conversion
    while( ( AD1CON1bits.DONE == 0 ) && ( m_sampleTimer->ms() < 10 ) ) {} // Wait for conversion to complete

    if( AD1CON1bits.DONE == 1 )
    {
        if( m_input == voltage )
        {
            ++m_voltageSampleIdx; if( m_voltageSampleIdx == numSamples ) m_voltageSampleIdx = 0;
            m_rawVoltageSamples[m_voltageSampleIdx] = ADC1BUF0;
            
            // Select next input
            AD1CHSbits.CH0SA = 10; // AN10 = battery current.
            m_input = current;
        }
        else
        {
            ++m_currentSampleIdx; if( m_currentSampleIdx == numSamples ) m_currentSampleIdx = 0;
            m_rawCurrentSamples[m_currentSampleIdx] = ADC1BUF0;

            // Select next input
            AD1CHSbits.CH0SA = 9; // AN9 = battery voltage.
            m_input = voltage;
            m_allSampled = true;
        }
    }
    else
    {
        LOG_DEBUG( "ADC conversion timed out!" );
    }

    AD1CON1bits.SAMP = 1; // Start sampling next
}

void BatteryMonitor::doMonitor()
{
    if( dataValid() )
    {
        m_validTimer->reset();

        switch( m_state )
        {
        case pending:
            // Start-up state
            if( fastCharging() || ( ::fabs( averageCurrent() ) > ocCurrent ) )
            {
                m_pendingTimer->reset();
            }
            else if( m_pendingTimer->ms() >= estimationSettlingDelay )
            {
                // Not fast charging and low current draw (switched off).
                // Estimate starting SOC from OCV.
                estimateCharge();
                if( extPower() )
                {
                    m_state = charging;
                }
                else
                {
                    m_state = discharging;
                }
            }
            // Else if fast charging, or running on battery with a high load, we
            // can't really estimate battery state of charge from the voltage.
            // Wait until we stop charging or the computer is turned off and
            // we'll at least then be less wrong (there is always a margin of
            // error calculating SoC from voltage).
            break;
        case charging:
            if( !extPower() )
            {
                m_state = discharging;
            }
            else if( !fastCharging() && ( averageVoltage() >= maxChargeVoltage ) )
            {
                // Reset state of charge to full capacity.
                m_charge = capacityAtMaxCharge;
                m_state = charged;
            }
            else
            {
                // Coulomb count up.
                integrateCurrent();
            }
            break;
        case charged:
            if( !extPower() )
            {
                m_state = discharging;
            }
            else if( fastCharging() )
            {
                m_state = charging;
            }
            break;
        case discharging:
            if( extPower() )
            {
                m_state = charging;
            }
            else
            {
                // Coulomb count down.
                integrateCurrent();
            }
            break;
        case battError:
            m_state = pending;
            m_pendingTimer->reset();
            break;
        default:
            break;
        }
    }
    // Else voltage is not stable, the battery is missing (and the charger is
    // sending probing pulses), or we're pulsing due to battery conditioning.
    // Don't do anything and wait until the battery is inserted or starts
    // fast charging.
    else if( m_validTimer->ms() >= validTimeout )
    {
        // Invalid data are expected when starting/stopping charging and
        // connecting/disconnecting external power. Wait 10s before treating
        // this as an error.
        m_state = battError;
    }

#ifdef LOG_BATTERY
    LiteStream stream;
    switch( m_state )
    {
    case pending:
        stream << "P,";
        break;
    case charging:
        stream << "c,";
        break;
    case charged:
        stream << "C,";
        break;
    case discharging:
        stream << "D,";
        break;
    case battError:
        stream << "X,";
        break;
    default:
        stream << "?,";
        break;
    }

    ++m_debugCount;
    if( m_debugCount == 2 )
    {
        stream << ( dataValid() ? "V," : "," )
            << ( extPower() ? "E," : "," )
            << ( fastCharging() ? "F," : "," )
            << averageVoltage() << ","
            << averageCurrent() << ","
            << m_charge;
        LOG_DEBUG( stream.str() );
        m_debugCount = 0;
    }
#endif
}

bool BatteryMonitor::dataValid()
{
    int lowestVoltage( 9999 );
    int highestVoltage( 0 );
    for( int i = 0; i < numSamples; ++i )
    {
        int voltage( voltageFromCounts( m_rawVoltageSamples[i] ) );
        if( voltage < lowestVoltage ) lowestVoltage = voltage;
        if( voltage > highestVoltage ) highestVoltage = voltage;
    }

    bool valid( ( ( highestVoltage - lowestVoltage ) < maxVoltageDelta ) );

    return valid;
}

int BatteryMonitor::voltageFromCounts( int counts )
{
    // 3.3V / 1024 = 3.223mV/count
    // Resistor divider Vout = 0.4Vin
    // Range = 0 - 8.23V (battery ~2.5 - 4.2V, but VBatt sees larger spikes from
    // charging IC when battery missing and charger is sending external voltage
    // to probe for battery presence).
    return( counts * 8 );
}

int BatteryMonitor::currentFromCounts( int counts )
{
    // Gain 20.12
    // I = V/R = ( x / 20.12 ) / 0.04
    // 3.3V / 1024 = 3.223 mV/count
    // 1 count = 4 mA
    // Range = +/- 2048mA
    return( ( counts - 512 ) * 4 );
}

int BatteryMonitor::averageVoltage()
{
    long sum( 0 );
    for( int i = 0; i < numSamples; ++i )
    {
        sum += voltageFromCounts( m_rawVoltageSamples[i] );
    }
    return( sum / numSamples );
}

int BatteryMonitor::averageCurrent()
{
    long sum( 0 );
    for( int i = 0; i < numSamples; ++i )
    {
        sum += currentFromCounts( m_rawCurrentSamples[i] );
    }
    return( sum / numSamples );
}

bool BatteryMonitor::extPower()
{
    return !PORTBbits.RB11;
}

bool BatteryMonitor::fastCharging()
{
    return !PORTBbits.RB13;
}

void BatteryMonitor::integrateCurrent()
{
    double current( averageCurrent() );
    double chargeDelta( current / samplesPerHour );

    if( ( m_state == charging ) && ( current < 0 ) )
    {
        m_charge -= chargeDelta; // Subtracting negative current is addition.
        if( m_charge > capacityAtMaxCharge ) m_charge = capacityAtMaxCharge;
    }
    else if( ( m_state == discharging ) && ( current > 0 ) )
    {
        m_charge -= chargeDelta;
        if( m_charge < 0 ) m_charge = 0;
    }
}

void BatteryMonitor::estimateCharge()
{
    int voltage( averageVoltage() );
    int lutIdx1 = ( voltage / 50 ) - 60; // See LUT definiton above.
    if( ( lutIdx1 >= 0 ) && ( lutIdx1 < 24 ) )
    {
        // Interpolate between LUT points.
        int lutIdx2 = lutIdx1 + 1;
        double voltageFrac = double( voltage % 50 ) / 50;
        int chargeRange( dischargeCurve[lutIdx2] - dischargeCurve[lutIdx1] );
        m_charge = ( cellCapacity / 100 ) * ( dischargeCurve[lutIdx1] + ( voltageFrac * chargeRange ) );

#ifdef LOG_BATTERY
        LiteStream stream;
        stream << "Estimate charge LUT1: " << lutIdx1
               << " LUT2: " << lutIdx2
               << " vfrac: " << voltageFrac
               << " crange: " << chargeRange
               << " Charge: " << m_charge;
        LOG_DEBUG( stream.str() );
#endif
    }
}

} // namespace Agape
