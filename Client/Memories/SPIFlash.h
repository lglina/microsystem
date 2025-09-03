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
    SPIFlash( SPIController& spiController, BusController& busController );

    virtual enum Type type();

    virtual int read( int addr, char* data, int len );
    virtual int write( int addr, const char* data, int len );
    virtual bool erase( int addr, int len );
    virtual bool erase();

    virtual int size();
    virtual int sectorSize();

    void readID( char* id );

private:
    void writeEnable();
    void writePage( int addr, const char* data, int len );
    void waitWrite();

    SPIController& m_spiController;
    BusController& m_busController;
};

} // namespace Memories

} // namespace Agape

#endif // AGAPE_MEMORIES_SPI_FLASH_H
