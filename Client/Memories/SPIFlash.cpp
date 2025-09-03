#include "Loggers/Logger.h"
#include "Memories/Memory.h"
#include "Memories/SPIFlash.h"
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
    const char chipEraseCommand( '\x60' );
    const char readIDCommand( '\x9f' );
} // Anonymous namespace

namespace Agape
{

namespace Memories
{

SPIFlash::SPIFlash( SPIController& spiController, BusController& busController ) :
  m_spiController( spiController ),
  m_busController( busController )
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
    int baseAddress( addr );
    while( lenWritten < len )
    {
        int sectorRemain( sectorSize() - ( baseAddress % sectorSize() ) );
        int lenRemain( len - lenWritten );
        int lenThisWrite( ( sectorRemain > lenRemain ) ? lenRemain : sectorRemain );
        writePage( addr + lenWritten, data + lenWritten, lenThisWrite );
        lenWritten += lenThisWrite;
    }

    return lenWritten;
}

bool SPIFlash::erase( int addr, int len )
{
    // FIXME: Ensure len is exactly one sector and is aligned!
    writeEnable();

    m_busController.write( BusAddresses::CSFlash, 0x00 );
    m_spiController.write( sectorEraseCommand );
    m_spiController.write( addr >> 16 );
    m_spiController.write( addr >> 8 );
    m_spiController.write( addr );
    m_busController.write( 0x00, 0x00 ); // De-assert CS to complete write.

    // FIXME: Delay needed?

    waitWrite();

    return true;
}

bool SPIFlash::erase()
{
    writeEnable();

    m_busController.write( BusAddresses::CSFlash, 0x00 );
    m_spiController.write( chipEraseCommand );
    m_busController.write( 0x00, 0x00 ); // De-assert CS to complete write.

    // FIXME: Delay needed?

    waitWrite();

    return true;
}

int SPIFlash::size()
{
    return 1048576;
}

int SPIFlash::sectorSize()
{
    return 4096;
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

    // FIXME: Delay needed?

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
#endif

    writeEnable();

    m_busController.write( BusAddresses::CSFlash, 0x00 );
    m_spiController.write( pageProgramCommand );
    m_spiController.write( addr >> 16 );
    m_spiController.write( addr >> 8 );
    m_spiController.write( addr );
    m_spiController.write( data, len );
    m_busController.write( 0x00, 0x00 ); // De-assert CS to complete write.

    // FIXME: Delay needed?

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
