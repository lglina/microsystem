#ifndef AGAPE_HASHES_SHA256_H
#define AGAPE_HASHES_SHA256_H

#include "Encryptors/Hash.h"
#include "sha256.hpp"

namespace Agape
{

namespace Hashes
{

class SHA256 : public Hash
{
public:
    SHA256();

    virtual void update( const char* data, int len );

    virtual int digestSize();
    virtual void finalise( char* digest );

    virtual void reset();

private:
    SHA256_CTX_BC m_ctx;
};

} // namespace Hashes

} // namespace Agape

#endif // AGAPE_HASHES_SHA256_H
