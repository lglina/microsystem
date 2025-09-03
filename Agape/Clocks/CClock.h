#ifndef AGAPE_C_CLOCK_H
#define AGAPE_C_CLOCK_H

#include "Clock.h"
#include "String.h"

namespace Agape
{

namespace Clocks
{

class C : public Clock
{
public:
    C();

    virtual String dateTime();
    virtual void fillDateTime( char* dateTime );

    virtual unsigned long long epochMS();
};

} // namespace Clocks

} // namespace Agape

#endif // AGAPE_C_CLOCK_H
