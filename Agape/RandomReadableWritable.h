#ifndef AGAPE_RANDOM_READABLE_WRITABLE_H
#define AGAPE_RANDOM_READABLE_WRITABLE_H

namespace Agape
{

class RandomReadableWritable
{
public:
    virtual ~RandomReadableWritable() {}

    /// Note: Implementors may return fewer than len bytes. This is not an
    /// error, and the caller should call read() again with the next offset from
    /// where the previous read finished. Alternatively, call readAll() to try
    /// to return all bytes in a single read.
    virtual int read( char* data, int offset, int len ) = 0;

    /// Note: Not all devices will support random writes. In this case,
    /// non-monotonic writes or writes not starting at zero offset will
    /// always return zero, and callers should detect and handle this.
    virtual int write( const char* data, int offset, int len ) = 0;

    virtual bool error() = 0;

    /// Tries to read exactly len bytes in a single read. If fewer (or zero)
    /// bytes are returned, it can be assumed that there was a read error
    /// (error() may return true to indicate this). This is a utility function
    /// to prevent copypasta if every caller were to implement their own
    /// (inevitably buggy) read loop to handle the "possibly-reads-less-than-
    /// requested" semantics of read(), as above.
    int readAll( char* data, int offset, int len );
};

} // namespace Agape

#endif // AGAPE_RANDOM_READABLE_WRITABLE_H
