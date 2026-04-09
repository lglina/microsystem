#include "Loggers/Logger.h"
#include "Memories/Memory.h"
#include "Memories/SPIFlash.h"
#include "Utils/LiteStream.h"
#include "Utils/StrToHex.h"
#include "BusAddresses.h"
#include "BusController.h"
#include "SPIController.h"

namespace
{
    const char pageProgramCommand( '\x02' );
    const char readCommand( '\x03' );
    const char rdsrCommand( '\x05' );
    const char wrenCommand( '\x06' );
    const char sectorEraseCommand( '\x20' );
    const char blockEraseCommand( '\xd8' );
    const char chipEraseCommand( '\x60' );
    const char readIDCommand( '\x9f' );
} // Anonymous namespace

namespace Agape
{

namespace Memories
{

SPIFlash::SPIFlash( SPIController& spiController,
                    BusController& busController,
                    int size,
                    int pageSize,
                    int sectorSize,
                    int eraseBlockSize,
                    int baseAddress ) :
  m_spiController( spiController ),
  m_busController( busController ),
  m_size( size ),
  m_pageSize( pageSize ),
  m_sectorSize( sectorSize ),
  m_eraseBlockSize( eraseBlockSize ),
  m_baseAddress( baseAddress )
{
}

enum Memory::Type SPIFlash::type()
{
    return Memory::flash;
}

int SPIFlash::read( int addr, char* data, int len )
{
    m_busController.write( BusAddresses::CSFlash, 0x00 );
    m_spiController.write( readCommand );
    addr += m_baseAddress;
    m_spiController.write( addr >> 16 );
    m_spiController.write( addr >> 8 );
    m_spiController.write( addr );

    int lenRead( m_spiController.read( data, len ) );

    m_busController.write( 0x00, 0x00 ); // De-assert CS to complete read.

    return lenRead;
}

int SPIFlash::write( int addr, const char* data, int len )
{
    int lenWritten( 0 );
    addr += m_baseAddress;
    while( lenWritten < len )
    {
        int pageRemain( m_pageSize - ( addr % m_pageSize ) );
        int lenRemain( len - lenWritten );
        int lenThisWrite( ( pageRemain > lenRemain ) ? lenRemain : pageRemain );
        writePage( addr + lenWritten, data + lenWritten, lenThisWrite );
        lenWritten += lenThisWrite;
    }

    return lenWritten;
}

bool SPIFlash::erase( int addr, int len )
{
    if( ( addr < 0 ) ||
        ( ( addr + len ) > m_size ) ||
        ( ( addr % m_sectorSize ) != 0 ) ||
        ( ( len % m_sectorSize ) != 0 ) )
    {
        return false;
    }

    addr += m_baseAddress;

    // See if we can use faster bulk erase commands.
    // FIXME: We can't assume chip erase is safe here, as we don't have the
    // total flash size, only the partition size.
    char eraseCommand( sectorEraseCommand );
    int eraseLength( m_sectorSize );
    if( ( ( addr % m_eraseBlockSize ) == 0 ) &&
        ( ( len % m_eraseBlockSize ) == 0 ) )
    {
        eraseCommand = blockEraseCommand;
        eraseLength = m_eraseBlockSize;
    }

    while( len > 0 )
    {
        writeEnable();
#ifdef LOG_SPI
        LiteStream stream;
        stream << "Erase flash at " << uintToHex( addr );
        LOG_DEBUG( stream.str() );
#endif
        m_busController.write( BusAddresses::CSFlash, 0x00 );
        m_spiController.write( eraseCommand );
        m_spiController.write( addr >> 16 );
        m_spiController.write( addr >> 8 );
        m_spiController.write( addr );
        m_busController.write( 0x00, 0x00 ); // De-assert CS to complete write.

        waitWrite();

        addr += eraseLength;
        len -= eraseLength;
    }

    return true;
}

bool SPIFlash::erase()
{
    return erase( 0, m_size );
}

int SPIFlash::size()
{
    return m_size;
}

int SPIFlash::pageSize()
{
    return m_pageSize;
}

int SPIFlash::sectorSize()
{
    return m_sectorSize;
}

int SPIFlash::eraseBlockSize()
{
    return m_eraseBlockSize;
}

void SPIFlash::readID( char* id )
{
    m_busController.write( BusAddresses::CSFlash, 0x00 );
    m_spiController.write( readIDCommand );
    m_spiController.read( id, 3 );
    m_busController.write( 0x00, 0x00 );
}

void SPIFlash::writeEnable()
{
#ifdef LOG_SPI
    LOG_DEBUG( "SPIFlash: Write enable" );
#endif
    m_busController.write( BusAddresses::CSFlash, 0x00 );
    m_spiController.write( wrenCommand );
    m_busController.write( 0x00, 0x00 );

#ifdef LOG_SPI
    LOG_DEBUG( "SPIFlash: Read SR" );
#endif
    m_busController.write( BusAddresses::CSFlash, 0x00 );
    m_spiController.write( rdsrCommand );
    char status( '\0' );
    m_spiController.read( &status, 1 );

    while( !( status & 0x02 ) )
    {
        // FIXME: Timeout.
#ifdef LOG_SPI
        LOG_DEBUG( "SPIFlash: Read SR" );
#endif
        m_spiController.read( &status, 1 );
    }
    m_busController.write( 0x00, 0x00 );
#ifdef LOG_SPI
    LOG_DEBUG( "SPIFlash: Write enable OK" );
#endif
}

void SPIFlash::writePage( int addr, const char* data, int len )
{
#ifdef LOG_SPI
    LOG_DEBUG( "SPIFlash: Start write cycle" );

    LiteStream stream;
    stream << "Write flash at " << uintToHex( addr );
    LOG_DEBUG( stream.str() );
#endif

    writeEnable();

    // addr is absolute here, not relative to m_baseAddress.
    m_busController.write( BusAddresses::CSFlash, 0x00 );
    m_spiController.write( pageProgramCommand );
    m_spiController.write( addr >> 16 );
    m_spiController.write( addr >> 8 );
    m_spiController.write( addr );
    m_spiController.write( data, len );
    m_busController.write( 0x00, 0x00 ); // De-assert CS to complete write.

    waitWrite();

#ifdef LOG_SPI
    LOG_DEBUG( "SPIFlash: Write cycle complete." );
#endif
}

void SPIFlash::waitWrite()
{
#ifdef LOG_SPI
    LOG_DEBUG( "SPIFlash: Wait: Read SR" );
#endif
    m_busController.write( BusAddresses::CSFlash, 0x00 );
    m_spiController.write( rdsrCommand );
    char status( '\0' );
    m_spiController.read( &status, 1 );

    while( status & 0x01 )
    {
        // FIXME: Timeout.
#ifdef LOG_SPI
        LOG_DEBUG( "SPIFlash: Wait: Read SR" );
#endif
        m_spiController.read( &status, 1 );
    }
    m_busController.write( 0x00, 0x00 );
}

} // namespace Memories

} // namespace Agape
