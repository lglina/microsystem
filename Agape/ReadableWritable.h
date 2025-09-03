#ifndef AGAPE_READABLE_WRITABLE_H
#define AGAPE_READABLE_WRITABLE_H

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class String;
class Timer;

class ReadableWritable
{
public:
    ReadableWritable();
    virtual ~ReadableWritable();

    virtual int read( char* data, int len, bool blocking, int timeout = 0 );
    virtual int read( char* data, int len ) = 0;
    virtual void readLine( String& s, int timeout = 0 );
    virtual int write( const char* data, int len ) = 0;
    virtual bool error() = 0;

    virtual void flushInput() {};
    virtual void flushOutput() {};

    static void setTimerFactory( Timers::Factory* timerFactory );

    static const bool rwBlock = true;
    static const bool rwNonBlock = false;

private:
    static Timers::Factory* s_timerFactory;
    Timer* m_timer;
};

} // namespace Agape

#endif // AGAPE_READABLE_WRITABLE_H
