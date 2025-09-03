#ifndef AGAPE_LOGGER_H
#define AGAPE_LOGGER_H

#include "String.h"

#define LOG_DEBUG( m ) Agape::Logger::getInstance()->log( m, Agape::Logger::debug )

/*
Logging enable/disable defines used:
LOG_LOADERS
LOG_CARLO_INTERP
LOG_CARLO_PARSER
LOG_TUPLES
LOG_TUPLES_BRIEF
LOG_WS
LOG_AESBLOCK
LOG_SPI
LOG_KEYB
LOG_BATTERY
LOG_WINDOWS
KIAMA_DEBUG
*/

namespace Agape
{

class Logger
{
public:
    enum LogLevel
    {
        error,
        warning,
        info,
        debug
    };

    static void setInstance( Logger* logger );
    static Logger* getInstance();

    virtual void log( const String& message, LogLevel logLevel );

protected:
    static Logger* s_instance;
};

} // namespace Agape

#endif // AGAPE_LOGGER_H
