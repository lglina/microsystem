#include "SHA256Hash.h"

namespace Agape
{

namespace Hashes
{

SHA256::SHA256()
{
    reset();
}

void SHA256::update( const char* data, int len )
{
    sha256_update( &m_ctx, (const BYTE*)data, len );
}

int SHA256::digestSize()
{
    return SHA256_BLOCK_SIZE;
}

void SHA256::finalise( char* digest )
{
    sha256_final( &m_ctx, (BYTE*)digest );
}

void SHA256::reset()
{
    sha256_init( &m_ctx );
}

} // namespace Hashes

} // namespace Agape
