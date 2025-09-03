#include "DummyEntropySource.h"

namespace Agape
{

namespace EntropySources
{

Dummy::Dummy() :
  m_nextChar( '\0' )
{
}

int Dummy::generate( char* buffer, int len )
{
    for( int i = 0; i < len; ++i )
    {
        buffer[i] = m_nextChar++;
    }

    return len;
}

void Dummy::run()
{
}

} // namespace EntropySources

} // namespace Agape
