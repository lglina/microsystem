#ifndef AGAPE_LINES_DIRECT_H
#define AGAPE_LINES_DIRECT_H

#include "Lines/Line.h"
#include "LineDrivers/LineDriver.h"
#include "Collections.h"
#include "Runnable.h"
#include "String.h"

namespace Agape
{

namespace Lines
{

class Direct : public Line
{
public:
    Direct( LineDriver& lineDriver, bool dialSetsLinkAddress = true );

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
    bool m_dialSetsLinkAddress;
    int m_readyDelayTimer;
    int m_ringingDelayTimer;
};

} // namespace Lines

} // namespace Agape

#endif // AGAPE_LINES_DIRECT_H
