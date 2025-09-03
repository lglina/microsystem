#include "Logger.h"
#include "String.h"

namespace Agape
{

Logger* Logger::s_instance = nullptr;

void Logger::setInstance( Logger* logger )
{
    s_instance = logger;
}

Logger* Logger::getInstance()
{
    if( !s_instance )
    {
        s_instance = new Logger;
    }
    return s_instance;
}

void Logger::log( const String& message, LogLevel logLevel )
{
}

} // namespace Agape
