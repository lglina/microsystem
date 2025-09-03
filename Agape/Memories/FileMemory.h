#ifndef AGAPE_MEMORIES_FILE_H
#define AGAPE_MEMORIES_FILE_H

#include "Memory.h"
#include "String.h"

#include <sys/types.h>

namespace Agape
{

namespace Memories
{

class File : public Memory
{
public:
    File( const String& filename, enum Type type, long createSize = 1048576, long sectorSize = 4096 );
    ~File();

    virtual enum Type type();

    virtual int read( int addr, char* data, int len );
    virtual int write( int addr, const char* data, int len );
    virtual bool erase( int addr, int len );

    virtual int size();
    virtual int sectorSize();

private:
    enum Type m_type;
    long m_sectorSize;

    int m_fd;
    off_t m_size;    
};

} // namespace Memories

} // namespace Agape

#endif // AGAPE_MEMORIES_FILE_H
