#ifndef AGAPE_LINE_DRIVERS_PIC_SERIAL_H
#define AGAPE_LINE_DRIVERS_PIC_SERIAL_H

#include "LineDrivers/LineDriver.h"
#include "PICSerial.h"

namespace Agape
{

namespace LineDrivers
{

class PICSerial : public LineDriver
{
public:
    PICSerial( Agape::PICSerial& picSerial );

    virtual int open();
    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );

    virtual bool dataCarrierDetect();
    virtual void dataTerminalReady( bool ready );

private:
    Agape::PICSerial& m_picSerial;
};

} // namespace LineDrivers

} // namespace Agape

#endif // AGAPE_LINE_DRIVERS_PIC_SERIAL_H
