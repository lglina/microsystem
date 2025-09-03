#ifndef AGAPE_LINES_MODEM_H
#define AGAPE_LINES_MODEM_H

#include "Lines/Line.h"
#include "LineDrivers/LineDriver.h"
#include "Collections.h"
#include "Runnable.h"
#include "String.h"

namespace Agape
{

namespace Lines
{

class Modem : public Line
{
public:
    Modem( LineDriver& lineDriver );

    virtual void open();
    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );

    virtual Vector< Line::ConfigOption > getConfigOptions();
    virtual void setConfigOption( const String& name, const String& value );

    virtual void connect();
    virtual void registerNumber( const String& number );

    virtual void dial( const String& number );
    virtual void answer();
    virtual void hangup();

    virtual struct Line::LineStatus getLineStatus();

private:
    bool m_isOpen;

    int m_txCounter;
    int m_rxCounter;
};

} // namespace Lines

} // namespace Agape

#endif // AGAPE_LINES_MODEM_H
