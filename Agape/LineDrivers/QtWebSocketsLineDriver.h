#ifndef AGAPE_LINE_DRIVERS_QT_WEBSOCKETS_H
#define AGAPE_LINE_DRIVERS_QT_WEBSOCKETS_H

#include "Network/QtWebSocketsConnection.h"
#include "LineDriver.h"
#include "String.h"

#include <memory>

namespace Agape
{

class Platform;

namespace LineDrivers
{

class QtWebSockets : public LineDriver
{
public:
    QtWebSockets( Platform& platform );

    virtual int open();
    virtual bool isSecure();

    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );
    virtual bool error();

    virtual void setLinkAddress( const String& number );
    virtual bool linkReady();

private:
    Platform& m_platform;

    String m_number;
    std::unique_ptr< Network::QtWebSocketsConnection > m_webSocketsConnection;
};

} // namespace LineDrivers

} // namespace Agape

#endif // AGAPE_LINE_DRIVERS_QT_WEBSOCKETS_H
