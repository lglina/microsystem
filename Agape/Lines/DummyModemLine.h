#ifndef AGAPE_LINES_DUMMY_MODEM_H
#define AGAPE_LINES_DUMMY_MODEM_H

#include "Lines/Line.h"
#include "Collections.h"
#include "String.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class ConfigurationStore;
class LineDriver;
class Timer;

namespace Lines
{

class DummyModem : public Line
{
public:
    DummyModem( LineDriver& lineDriver,
                ConfigurationStore& configurationStore,
                Timers::Factory& timerFactory,
                bool dialSetsLinkAddress = true );
    ~DummyModem();

    virtual void run();

    virtual Vector< Line::ConfigOption > getConfigOptions();
    virtual void setConfigOption( const String& name, const String& value );

    virtual void connect();
    virtual void registerNumber( const String& number );

    virtual void dial( const String& number );
    virtual void answer();
    virtual void hangup();

    virtual struct Line::LineStatus getLineStatus();

private:
    enum State
    {
        disconnected,
        connecting,
        connected,
        dialling,
        carrier
    };

    void loadAccessPoints();

    ConfigurationStore& m_configurationStore;
    Timer* m_timer;

    bool m_dialSetsLinkAddress;
    
    enum State m_state;

    bool m_accessPointsLoaded;
    Vector< String > m_accessPointsAvailable;
    Vector< String > m_accessPointsAdded;

    String m_addName;
};

} // namespace Lines

} // namespace Agape

#endif // AGAPE_LINES_DUMMY_MODEM_H
