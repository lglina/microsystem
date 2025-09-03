#ifndef AGAPE_MEMORIES_KIAMA_FS_H
#define AGAPE_MEMORIES_KIAMA_FS_H

#include "KiamaFS.h"
#include "Memory.h"
#include "String.h"

namespace Agape
{

namespace Memories
{

class KiamaFS : public Memory
{
public:
    KiamaFS( Agape::KiamaFS& fs, const String& filename );

    virtual enum Type type();

    virtual int read( int addr, char* data, int len );
    virtual int write( int addr, const char* data, int len );
    virtual bool erase( int addr, int len );
    virtual void seek( int offset );

    virtual int size();
    virtual int sectorSize();

    virtual void flushOutput();

private:
    Agape::KiamaFS& m_fs;
    String m_filename;
    bool m_isOpen;
    bool m_writeMode;
    Agape::KiamaFS::File* m_file;
};

} // namespace Memories

} // namespace Agape

#endif // AGAPE_MEMORIES_KIAMA_FS_H
