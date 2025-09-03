#include "DevRandom.h"

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

namespace Agape
{

namespace EntropySources
{

DevRandom::DevRandom()
{
    m_fd = ::open( "/dev/random", O_RDONLY );
}

int DevRandom::generate( char* buffer, int len )
{
    return ::read( m_fd, buffer, len );
}

void DevRandom::run()
{
}

} // namespace EntropySources

} // namespace Agape
