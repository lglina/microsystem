#ifndef AGAPE_LINE_DRIVERS_PIC_SERIAL_H
#define AGAPE_LINE_DRIVERS_PIC_SERIAL_H

#include "LineDrivers/LineDriver.h"
#include "PICSerial.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class Timer;

namespace LineDrivers
{

class PICSerial : public LineDriver
{
public:
    PICSerial( Agape::PICSerial& picSerial, Timers::Factory& timerFactory );
    virtual ~PICSerial();

    virtual int open();
    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );
    virtual bool error();

private:
    Agape::PICSerial& m_picSerial;

    Timer* m_rxTimer;
};

} // namespace LineDrivers

} // namespace Agape

#endif // AGAPE_LINE_DRIVERS_PIC_SERIAL_H
