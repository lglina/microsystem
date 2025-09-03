#include "UI/VRTime.h"
#include "Utils/LiteStream.h"
#include "VRTimeClock.h"
#include "String.h"

#include <string.h>

namespace Agape
{

namespace Clocks
{

VRTime::VRTime( UI::VRTime& vrTimeUI ) :
  m_vrTimeUI( vrTimeUI )
{
}

String VRTime::dateTime()
{
    String dateTime;
    timestampToISO( m_vrTimeUI.m_secsSinceEpoch, dateTime );
    return dateTime;
}

void VRTime::fillDateTime( char* fdateTime )
{
    ::strncpy( fdateTime, dateTime().c_str(), 14 );
}

unsigned long long VRTime::epochMS()
{
    m_vrTimeUI.waitForTime();

    return (unsigned long long)m_vrTimeUI.m_secsSinceEpoch * 1000ull;
}

} // namespace Clocks

} // namespace Agape
