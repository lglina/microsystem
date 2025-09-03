#ifndef AGAPE_PLATFORMS_SIMULATED_H
#define AGAPE_PLATFORMS_SIMULATED_H

#include "Platforms/Platform.h"
#include "Collections.h"
#include "String.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class KiamaFS;
class Timer;

namespace Platforms
{

class Simulated : public Platform
{
public:
    Simulated( Timers::Factory& timerFactory, KiamaFS* fs = nullptr );
    ~Simulated();

    virtual void performSelfTest();

    virtual void doBootTasks();

    virtual bool error();
    virtual void currentErrors( Vector< enum ErrorType >& errors );

    virtual long heapUsed();

    virtual struct Platform::PowerState powerState();

    virtual void notify( enum NotifyType type, enum NotifySource source );
    virtual void cancelNotify( enum NotifyType type );

    virtual void run();

private:
    KiamaFS* m_fs;
    Timer* m_timer;
};

} // namespace Platforms

} // namespace Agape

#endif // AGAPE_PLATFORMS_SIMULATED_H
