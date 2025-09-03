#ifndef AGAPE_MEMORY_H
#define AGAPE_MEMORY_H

#include "ReadableWritable.h"

namespace Agape
{

class Memory : public ReadableWritable
{
public:
    enum Type
    {
        flash,
        eeprom
    };

    Memory();

    virtual enum Type type() = 0;

    virtual int read( int addr, char* data, int len ) = 0;
    virtual int write( int addr, const char* data, int len ) = 0;
    virtual bool erase( int addr, int len ) = 0;
    virtual bool erase();

    virtual int read( char* data, int len );
    virtual int write( const char* data, int len );
    virtual bool error();
    virtual void seek( int offset );

    virtual int size() = 0;
    virtual int sectorSize() = 0;

protected:
    int m_rwAddress;
};

} // namespace Agape

#endif // AGAPE_MEMORY_H
