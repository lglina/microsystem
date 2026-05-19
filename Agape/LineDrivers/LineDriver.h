#ifndef AGAPE_LINE_DRIVER_H
#define AGAPE_LINE_DRIVER_H

#include "ReadableWritable.h"
#include "Runnable.h"

#ifdef QT_CORE_LIB
#include <QObject>
#endif // QT_CORE_LIB

namespace Agape
{

class String;

#ifdef QT_CORE_LIB
// FIXME: Do any derived classes still use Qt?
class LineDriver : public QObject, public ReadableWritable, public Runnable
{
#else
class LineDriver : public ReadableWritable, public Runnable
{
#endif // QT_CORE_LIB
public:
    virtual ~LineDriver();

    virtual int open() = 0;
    virtual void reset();

    virtual bool isSecure();

    virtual int read( char* data, int len ) = 0;
    virtual int write( const char* data, int len ) = 0;
    virtual bool error();

    virtual void setLinkAddress( const String& number );
    virtual bool linkReady();

    virtual void enableFlowControl( bool enable );
    virtual int controlLines();
    virtual void setControlLines( int mask );
    virtual void clearControlLines( int mask );

    // Convenience functions.
    bool dataCarrierDetect();
    void dataTerminalReady( bool ready );

    virtual void run();
};

} // namespace Agape

#endif // AGAPE_LINE_DRIVER_H
