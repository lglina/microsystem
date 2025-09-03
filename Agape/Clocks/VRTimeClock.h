#ifndef AGAPE_CLOCKS_VR_TIME_H
#define AGAPE_CLOCKS_VR_TIME_H

#include "Clock.h"
#include "String.h"

namespace Agape
{

namespace UI
{
class VRTime;
} // namespace UI

namespace Clocks
{

class VRTime : public Clock
{
public:
    VRTime( UI::VRTime& vrTimeUI );

    virtual String dateTime();
    virtual void fillDateTime( char* fdateTime );

    virtual unsigned long long epochMS();

private:
    UI::VRTime& m_vrTimeUI;
};

} // namespace Clocks

} // namespace Agape

#endif // AGAPE_CLOCKS_VR_TIME_H
