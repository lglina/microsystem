#ifndef AGAPE_ENCRYPTORS_AES_BLOCK_H
#define AGAPE_ENCRYPTORS_AES_BLOCK_H

#include "Encryptors/BlockEncryptor.h"
#include "RandomReadableWritable.h"
#include "String.h"

#include "Encryptors/AES/tiny-AES-c/aes.hpp"

#include <cstdint>

namespace Agape
{

class EntropySource;

namespace Encryptors
{

class AESBlock : public BlockEncryptor
{
public:
    AESBlock( EntropySource& entropySource );
    ~AESBlock();

    virtual void setKey( const char* key );

    virtual bool open( enum OpenMode openMode, RandomReadableWritable* backingDevice );

    virtual int read( char* data, int offset, int len );

    /// Writes must begin at zero offset and offsets must increase
    /// monotonically. Must call close() when done to write any remaining
    /// buffered partial block of ciphertext, if any.
    virtual int write( const char* data, int offset, int len );

    virtual bool error();

    virtual void close();

    virtual int overhead();

private:
    void calculateIV( int offset, std::uint8_t* blockiv );
    void initForBlock( struct AES_ctx& context, int offset );
    void flushWriteBuffer();

    EntropySource& m_entropySource;
    std::uint8_t m_key[AES_BLOCKLEN];
    std::uint8_t m_iv[AES_BLOCKLEN];

    char* m_writeBuffer;
    int m_writeBufferFill;
    int m_writeBufferOffset;
};

} // namespace Encryptors

} // namespace Agape

#endif // AGAPE_ENCRYPTORS_AES_BLOCK_H
