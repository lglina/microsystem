#ifndef AGAPE_MEMORIES_RAM_H
#define AGAPE_MEMORIES_RAM_H

#include "Memory.h"

namespace Agape
{

class AssetLoader;

namespace Memories
{

class RAM : public Memory
{
public:
    RAM( int size, int sectorSize, enum Type type );
    ~RAM();

    virtual enum Type type();

    virtual int read( int addr, char* data, int len );
    virtual int write( int addr, const char* data, int len );
    virtual bool erase( int addr, int len );

    virtual int size();
    virtual int sectorSize();

    void loadFromAsset( AssetLoader& assetLoader );

private:
    int m_size;
    int m_sectorSize;
    enum Type m_type;
    char* m_data;
};

} // namespace Memories

} // namespace Agape

#endif // AGAPE_MEMORIES_RAM_H
