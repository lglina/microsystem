#ifndef AGAPE_INPUT_DEVICE_H
#define AGAPE_INPUT_DEVICE_H

#include "../Runnable.h"

#ifdef QT_CORE_LIB
#include <QObject>
#endif // QT_CORE_LIB

namespace Agape
{

#ifdef QT_CORE_LIB
class InputDevice : public Runnable, public QObject
{
#else
class InputDevice : public Runnable
{
#endif // QT_CORE_LIB

public:
    // FIXME: Need to make this more like ReadableWritable?
    // Some input devices can't peek the buffer, so eof() can't be const :(
    virtual bool eof() = 0;
    virtual char peek() = 0;
    virtual char get() = 0;
};

namespace InputDevices
{
inline char control( char c )
{
    return( c - 0x80 );
}

namespace Key
{
const char carriageReturn( '\r' );
const char newLine( '\n' );
const char backspace( '\x08' );
const char tab( '\t' );
const char escape( '\x1b' );
const char up( '\x80' );
const char down( '\x81' );
const char left( '\x82' );
const char right( '\x83' );
const char shiftTab( '\x84' );
const char shiftUp( '\x85' );
const char shiftDown( '\x86' );
const char shiftLeft( '\x87' );
const char shiftRight( '\x88' );
}

} // namespace InputDevices

} // namespace Agape

#endif // AGAPE_INPUT_DEVICE_H
