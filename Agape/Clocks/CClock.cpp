#include "CClock.h"
#include "String.h"

#include <chrono>

#include <cstring>
#include <ctime>

using namespace std::chrono;

namespace Agape
{

namespace Clocks
{

C::C()
{
}

String C::dateTime()
{
    char formattedTime[15];
    fillDateTime( formattedTime );
    formattedTime[14] = '\0';
    return formattedTime;
}

void C::fillDateTime( char* dateTime )
{
    // FIXME: Use std::chrono?
    std::time_t t = std::time( nullptr );
    char temp[15];
    std::strftime( temp, 15, "%Y%m%d%H%M%S", std::localtime( &t ) );
    ::memcpy( dateTime, temp, 14 );
}

unsigned long long C::epochMS()
{
    return( duration_cast< milliseconds >( system_clock::now().time_since_epoch() ).count() );
}

} // namespace Clocks

} // namespace Agape
