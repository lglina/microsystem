#ifndef AGAPE_MEMORIES_SPI_FLASH_H
#define AGAPE_MEMORIES_SPI_FLASH_H

#include "Memories/Memory.h"

namespace Agape
{

class BusController;
class SPIController;

namespace Memories
{

class SPIFlash : public Memory
{
public:
    SPIFlash( SPIController& spiController,
              BusController& busController,
              int size,
              int pageSize,
              int sectorSize,
              int eraseBlockSize,
              int baseAddress );

    virtual enum Type type();

    virtual int read( int addr, char* data, int len );
    virtual int write( int addr, const char* data, int len );
    virtual bool erase( int addr, int len );
    virtual bool erase();

    virtual int size();
    virtual int pageSize();
    virtual int sectorSize();
    virtual int eraseBlockSize();

    void readID( char* id );

private:
    void writeEnable();
    void writePage( int addr, const char* data, int len );
    void waitWrite();

    SPIController& m_spiController;
    BusController& m_busController;

    int m_size;
    int m_pageSize;
    int m_sectorSize;
    int m_eraseBlockSize;
    int m_baseAddress;
};

} // namespace Memories

} // namespace Agape

#endif // AGAPE_MEMORIES_SPI_FLASH_H
