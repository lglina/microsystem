#ifndef AGAPE_LINE_DRIVERS_NULL_H
#define AGAPE_LINE_DRIVERS_NULL_H

#include "LineDriver.h"

namespace Agape
{

namespace LineDrivers
{

class Null : public LineDriver
{
public:
    Null();

    virtual int open();
    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );

    virtual void setLinkAddress();
    virtual bool linkReady();

private:
    bool m_linkReady;
};

} // namespace LineDrivers

} // namespace Agape

#endif // AGAPE_LINE_DRIVERS_NULL_H
