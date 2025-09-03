#ifndef AGAPE_BLOCK_ENCRYPTOR_H
#define AGAPE_BLOCK_ENCRYPTOR_H

#include "RandomReadableWritable.h"

namespace Agape
{

class BlockEncryptor : public RandomReadableWritable
{
public:
    enum OpenMode
    {
        modeNone,
        modeRead,
        modeWrite
    };

    BlockEncryptor();

    virtual void setKey( const char* key ) = 0;

    /// Open the backing device. May initialise the cryptosystem in a device-
    /// specific way (e.g. for AES CTR mode, read the IV from the start of
    /// the backing device to initialise the block counter).
    virtual bool open( enum OpenMode openMode, RandomReadableWritable* backingDevice ) = 0;

    /// read() operations may return any number of bytes up to "len" depending
    /// on the implementation buffer size and whether offsets are aligned to the
    /// underlying block size.
    virtual int read( char* data, int offset, int len ) = 0;

    /// Per the RandomReadableWritable interface, the implementation may require
    /// that writes begin at zero offset and are monotonic. This will be the
    /// case for blobk encryption (i.e. AES), as we must encrypt whole blocks
    /// and cannot encrypt partial blocks, or blocks at arbitrary offsets.
    virtual int write( const char* data, int offset, int len ) = 0;

    virtual bool error() = 0;

    virtual void close() = 0;

    virtual int overhead() = 0;

protected:
    RandomReadableWritable* m_backingDevice;
    enum OpenMode m_openMode;
};

} // namespace Agape

#endif // AGAPE_BLOCK_ENCRYPTOR_H
