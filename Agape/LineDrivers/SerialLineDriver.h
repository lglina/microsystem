#ifndef AGAPE_LINE_DRIVERS_SERIAL_H
#define AGAPE_LINE_DRIVERS_SERIAL_H

#include "LineDrivers/LineDriver.h"

#include "String.h"

namespace Agape
{

namespace LineDrivers
{

class Serial : public LineDriver
{
public:
    Serial( const String& device, int baudRate );

    virtual int open();
    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );

private:
    String m_device;
    int m_baudRate;

    int m_port;

    bool m_isOpen;
};

} // namespace LineDrivers

} // namespace Agape

#endif // AGAPE_LINE_DRIVER_H
