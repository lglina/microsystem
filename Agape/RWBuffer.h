#ifndef AGAPE_RW_BUFFER_H
#define AGAPE_RW_BUFFER_H

#include "ReadableWritable.h"

namespace Agape
{

/// Buffers writes up to "size" bytes before writing to underlying device.
/// Partial buffers should be flush()'ed. Reads are simply passed through.
class RWBuffer : public ReadableWritable
{
public:
    RWBuffer( int size, ReadableWritable& rw );
    virtual ~RWBuffer();

    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );
    virtual bool error();
    virtual void flushOutput();

private:
    int m_size;
    ReadableWritable& m_rw;
    
    int m_offset;
    char* m_buffer;
};

} // namespace Agape

#endif // AGAPE_RW_BUFFER_H
