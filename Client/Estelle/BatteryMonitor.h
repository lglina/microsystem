#ifndef AGAPE_BATTERY_MONITOR_H
#define AGAPE_BATTERY_MONITOR_H

#include "Runnable.h"
#include "SPIResponder.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class Timer;

class BatteryMonitor : public SPIResponseSource, public Runnable
{
public:
    enum State
    {
        unknown, // Not used here, but for commonality with Platform.h
        pending,
        charging,
        charged,
        discharging,
        battError
    };

    BatteryMonitor( Timers::Factory& timerFactory );
    ~BatteryMonitor();

    virtual int spiResponse( char requestType,
                             char* request,
                             int requestLength,
                             char* response,
                             int maxResponseLength );

    virtual void run();

private:
    enum Input
    {
        voltage,
        current
    };

    void doSample();
    void doMonitor();

    bool dataValid();

    int voltageFromCounts( int counts );
    int currentFromCounts( int counts );

    int averageVoltage();
    int averageCurrent();

    bool extPower();
    bool fastCharging();

    void integrateCurrent();
    void estimateCharge();

    Timer* m_sampleTimer;
    Timer* m_pendingTimer;
    Timer* m_validTimer;

    enum State m_state;

    double m_charge;

    int m_rawVoltageSamples[50];
    int m_rawCurrentSamples[50];
    
    int m_voltageSampleIdx;
    int m_currentSampleIdx;

    bool m_allSampled;

    enum Input m_input;

    int m_debugCount;
};

} // namespace Agape

#endif // AGAPE_BATTERY_MONITOR_H
