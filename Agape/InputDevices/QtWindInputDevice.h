#ifndef AGAPE_INPUT_DEVICES_QTWIND_H
#define AGAPE_INPUT_DEVICES_QTWIND_H

#include "Collections.h"
#include "InputDevice.h"
#include "QKeyEvent"
#include <QObject>

namespace Agape
{

namespace InputDevices
{

class QtWind : public InputDevice
{
    Q_OBJECT

public:
    virtual bool eof();
    virtual char peek();
    virtual char get();

    virtual void run();

private:
    Deque< char > m_buffer;

public slots:
    virtual void consumeKeyPress( QKeyEvent* event );
};

} // namespace InputDevices

} // namespace Agape

#endif // AGAPE_INPUT_DEVICES_QTWIND_H