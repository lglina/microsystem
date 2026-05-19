#include "Lines/Line.h"
#include "LineDriver.h"
#include "String.h"

namespace Agape
{

LineDriver::~LineDriver()
{
    // NOP
}

void LineDriver::reset()
{
    // NOP
}

bool LineDriver::isSecure()
{
    return false;
}

bool LineDriver::error()
{
    // TODO: Return errors?
    return false;
}

void LineDriver::setLinkAddress( const String& number )
{
    // NOP
}

bool LineDriver::linkReady()
{
    return false;
}

void LineDriver::enableFlowControl( bool enable )
{
    // NOP
}

int LineDriver::controlLines()
{
    return 0;
}

void LineDriver::setControlLines( int mask )
{
    // NOP
}

void LineDriver::clearControlLines( int mask )
{
    // NOP
}

bool LineDriver::dataCarrierDetect()
{
    return( ( controlLines() & Line::DCD ) == Line::DCD );
}

void LineDriver::dataTerminalReady( bool ready )
{
    ready ? setControlLines( Line::DTR ) : clearControlLines( Line::DTR );
}

void LineDriver::run()
{
    // NOP
}

} // namespace Agape
