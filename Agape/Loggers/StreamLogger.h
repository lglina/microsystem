#ifndef AGAPE_LOGGERS_STREAM_H
#define AGAPE_LOGGERS_STREAM_H

#include "Loggers/Logger.h"
#include "String.h"

namespace Agape
{

namespace Loggers
{

class Stream : public Logger
{
public:
    virtual void log( const String& message, LogLevel logLevel );
};

} // namespace Loggers

} // namespace Agape

#endif // AGAPE_LOGGERS_STREAM_H
