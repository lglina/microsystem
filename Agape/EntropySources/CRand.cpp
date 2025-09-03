#include "CRand.h"

#include <cstdlib>

namespace Agape
{

namespace EntropySources
{

int CRand::generate( char* buffer, int len )
{
    for( int i = 0; i < len; ++i )
    {
        *buffer++ = rand();
    }

    return len;
}

void CRand::run()
{
}

} // namespace EntropySources

} // namespace Agape
