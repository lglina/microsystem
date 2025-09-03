#ifndef AGAPE_LOGGERS_SERIAL_H
#define AGAPE_LOGGERS_SERIAL_H

#include "Loggers/Logger.h"
#include "String.h"

namespace Agape
{

class PICSerial;

namespace Loggers
{

class Serial : public Logger
{
public:
    Serial( PICSerial& picSerial );

    virtual void log( const String& message, LogLevel logLevel );

private:
    PICSerial& m_picSerial;
};

} // namespace Loggers

} // namespace Agape

#endif // AGAPE_LOGGERS_SERIAL_H
