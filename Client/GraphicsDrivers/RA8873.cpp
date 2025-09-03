#include "GraphicsDrivers/ARGBColours.h"
#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/Cartesian.h"
#include "Utils/LiteStream.h"
#include "Utils/StrToHex.h"
#include "BusAddresses.h"
#include "BusController.h"
#include "RA8873.h"
#include "String.h"
#include "vga.h"
#include "Avatars.h"

#include <string.h>

#include <xc.h>

using Agape::String;

extern const unsigned char vgaGlyphs[4096];

namespace
{
    const int _height( 480 );
    const int _width( 800 );
    const int _glyphHeight( 16 );
    const int _glyphWidth( 8 );

    /* Required Notice: */ const String copyright( "(C) Lauren Glina 2019-2025" );

    // For panic screen
    const int sadAgapeHeight( 8 );
    const int sadAgapeWidth( 14 );
    const unsigned char sadAgape[sadAgapeHeight][sadAgapeWidth] =
    {
        0x00, 0x00, 0x00, 0x00, 0xdc, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xb2, 0xb1, 0x00, 0x00, 0x00, 0xb2, 0xb1, 0x00, 0x00, 0x00,
        0x00, 0xdc, 0xdc, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdc, 0x00, 0x00,
        0xb2, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xb2, 0xb1, 0xb2,
        0xdf, 0xdb, 0xdb, 0xb2, 0xdc, 0xdb, 0xdb, 0xdb, 0xb2, 0xdc, 0xdb, 0xb2, 0xb1, 0x00,
        0x00, 0xdf, 0xdb, 0xdb, 0xdb, 0xdf, 0xdf, 0xdf, 0xdb, 0xdb, 0xb2, 0xb1, 0x00, 0x00,
        0x00, 0x00, 0xdf, 0xdb, 0xdc, 0xdb, 0xdb, 0xdb, 0xdc, 0xb2, 0xdf, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xdf, 0xdb, 0xdb, 0xb2, 0xdf, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    const unsigned char sadAgapeAttributes( 0x08 );
    const int panicWindowX = 144;
    const int panicWindowY = 144;
    const int panicWindowHeight = 176;
    const int panicWindowWidth = 512;
    const int sadAgapeRow = 1;
    const int sadAgapeCol = 5;
    const int panicMessageFirstRow = 2;
    const int panicMessageFirstCol = 24;
    const char* panicMessage1 = "Your computer has gone wrong.";
    const char* panicMessage2 = "Press and hold Esc to restart.";
    const char* panicMessage3 = "An instruction - meditate upon:";

    const int timeout = 100; // ms
} // Anonymous namespace

namespace Agape
{

namespace GraphicsDrivers
{

RA8873::RA8873( BusController& bus, Timers::Factory& timerFactory ) :
  m_bus( bus ),
  m_screenRect( 0, 0, 480, 800 ),
  m_currentWindow( nullptr ),
  m_absX( -1 ),
  m_absY( -1 ),
  m_attribute( 0 ),
  m_transparency( false ),
  m_brightness( 0x7FFF ),
  m_winSwitches( 0 ),
  m_attrSwitches( 0 ),
  m_transSwitches( 0 ),
  m_posSwitches( 0 ),
  m_warn( 0 ),
  m_warnCount( 0 ),
  m_doLog( false )
{
    m_timer = timerFactory.makeTimer();

    LOG_DEBUG( "LCD: Initialising" );

    // Pre-requisite: PMP (m_bus) must be configured to respect RA8873M cycle
    // timings (Datasheet Table 7-2), Claire chip timings and Claire /CS and
    // RS to RA8873M setup and hold times. This will be the case with
    // PBCLK at 8 or 32MHz without any extra PMP wait states, however the PMP
    // may still need to be configured with extra wait states with regard to
    // cable lengths, signal reflections etc.

    reset();

    setPLLs();

    initRAM();

    // Display configuration.
    writeRegister( 0x10, 0x04 ); // Main image 16bpp.

    writeRegister( 0x12, 0x40 ); // Display on.

    writeRegister( 0x14, 0x63 ); // Set horizontal display width to 800px.
    writeRegister( 0x15, 0x00 );
    
    writeRegister( 0x1A, 0xDF ); // Set vertical display height to 480px.
    writeRegister( 0x1B, 0x01 );
    
    // Display main image and main window.
    writeRegister( 0x20, 0x00 ); // Set main image start address
    writeRegister( 0x21, 0x00 ); // to 0x00180000 (first block).
    writeRegister( 0x22, 0x18 );
    writeRegister( 0x23, 0x00 );

    writeRegister( 0x24, 0x20 ); // Set main image width to 800px.
    writeRegister( 0x25, 0x03 );

    writeRegister( 0x26, 0x00 ); // Set main window upper left X coords.
    writeRegister( 0x27, 0x00 );
    writeRegister( 0x28, 0x00 ); // Set main window upper left Y coords.
    writeRegister( 0x29, 0x00 );

    // Geometry engine configuration.
    writeRegister( 0x50, 0x00 ); // Set canvas start address
    writeRegister( 0x51, 0x00 ); // to 0x00180000 (first block).
    writeRegister( 0x52, 0x18 );
    writeRegister( 0x53, 0x00 );

    writeRegister( 0x54, 0x20 ); // Set canvas width to 800px.
    writeRegister( 0x55, 0x03 );

    writeRegister( 0x56, 0x00 ); // Set active window upper left X coords.
    writeRegister( 0x57, 0x00 );
    writeRegister( 0x58, 0x00 ); // Set active window upper left Y coords.
    writeRegister( 0x59, 0x00 );

    writeRegister( 0x5A, 0x20 ); // Set active window width to 800px.
    writeRegister( 0x5B, 0x03 );

    writeRegister( 0x5C, 0xE0 ); // Set active window height to 480px.
    writeRegister( 0x5D, 0x01 );

    writeRegister( 0x5E, 0x01 ); // Set canvas depth to 16bpp and set block mode.

    writeRegister( 0x85, 0x02 ); // Set XPWM[0] to output counter 0.

    setBrightness();

    writeRegister( 0x8A, 0xFF ); // Set TCNTB0 to 0x7FFF.
    writeRegister( 0x8B, 0x7F );

    writeRegister( 0x86, 0x23 ); // Start counter 0.

    clearAll();

    loadCGRAM();

    // Display copyright message. This must not be removed or changed,
    // or the computer will not function correctly.
    displayCopyright();

    checkStatus();

    LOG_DEBUG( "LCD: Initialisation complete" );
}

RA8873::~RA8873()
{
    delete( m_timer );
}

void RA8873::clearScreen( const String& windowName )
{
    const Window* thisWindow;
    if( findWindow( windowName, thisWindow ) )
    {
        clearRegion( thisWindow->m_rect );
        checkStatus();
    }
}

void RA8873::clearLines( const String& windowName, int from, int len )
{
    const Window* thisWindow;
    if( findWindow( windowName, thisWindow ) )
    {
        clearRegion( Rectangle( thisWindow->m_rect.originX(),
                                thisWindow->m_rect.originY() + ( from * _glyphHeight ),
                                len * _glyphHeight,
                                thisWindow->m_rect.width() ) );
        checkStatus();
    }
}

void RA8873::clearAll()
{
    clearRegion( m_screenRect );
    checkStatus();
}

void RA8873::paintGlyph( const String& windowName, int row, int col, char* glyphAttr, bool transparency, char charset )
{

    if( m_currentWindow == nullptr || m_currentWindow->m_name != windowName )
    {
        if( !findWindow( windowName, m_currentWindow ) )
        {
            m_currentWindow = nullptr;
        }
        
        ++m_winSwitches;
    }

    if( m_currentWindow != nullptr )
    {
        int x( col * _glyphWidth );
        int y( row * _glyphHeight );

        Rectangle glyphRect( m_currentWindow->m_rect.originX() + x,
                             m_currentWindow->m_rect.originY() + y,
                             _glyphHeight,
                             _glyphWidth );
        Rectangle clipRect( glyphRect.clipTo( m_screenRect ) );
        if( ( clipRect.height() == _glyphHeight ) &&
            ( clipRect.width() == _glyphWidth ) )
        {
            if( isWindowTopmostVisible( Rectangle( m_currentWindow->m_rect.originX() + x,
                                                   m_currentWindow->m_rect.originY() + y,
                                                   _glyphHeight,
                                                   _glyphWidth ),
                                                   m_currentWindow->m_name ) )
            {
                unsigned char newAttribute( *( (unsigned char*)glyphAttr + 1 ) );
                if( m_attribute != newAttribute )
                {
                    setColours( newAttribute );
                    m_attribute = newAttribute;
                    ++m_attrSwitches;
                }

                if( transparency && ( ( newAttribute >> 4 ) != 0 ) )
                {
                    transparency = false; // Don't blit if background non-black.
                }

                if( m_transparency != transparency )
                {
                    setTransparency( transparency );
                    m_transparency = transparency;
                    ++m_transSwitches;
                }

                int absX( m_currentWindow->m_rect.originX() + ( col * _glyphWidth ) );
                int absY( m_currentWindow->m_rect.originY() + ( row * _glyphHeight ) );
                if( ( absX != m_absX ) || ( absY != m_absY ) )
                {
                    // 63, 64, 65, 66 for text position...
                    setTextPosition( absX, absY );

                    m_absX = absX;
                    m_absY = absY;

                    ++m_posSwitches;
                }

                // All character codes are 16b. < 0x8000 treated as half-width (i.e. half of height).
                int glyphIdx( *( (unsigned char*)glyphAttr ) );
                if( charset == 1 ) glyphIdx += 128;
                m_bus.write( BusAddresses::GraphCommand, 0x04 ); // Data port.
                writeData( glyphIdx >> 8 );
                writeData( glyphIdx & 0xFF );

                m_absX += _glyphWidth;
                if( m_absX >= _width )
                {
                    m_absX -= _width;
                    m_absY += _glyphHeight;
                    if( m_absY >= _height )
                    {
                        m_absY -= _height;
                    }
                }
            }
        }
    }

    checkStatus();
}

int RA8873::glyphHeight()
{
    return _glyphHeight;
}

int RA8873::glyphWidth()
{
    return _glyphWidth;
}

void RA8873::brightnessUp()
{
    m_brightness += 0x1000;
    if( m_brightness > 0x7FFF ) m_brightness = 0x7FFF;
    setBrightness();
}

void RA8873::brightnessDown()
{
    m_brightness -= 0x1000;
    if( m_brightness < 0x1000 ) m_brightness = 0x1000;
    setBrightness();
}

bool RA8873::error()
{
    return( m_warnCount >= 100 );
}

int RA8873::selfTest()
{
    if( m_doLog ) LOG_DEBUG( "LCD: Started self test" );

    waitForCoreIdle();

    writeRegister( 0x03, 0x00 ); // Set graphics mode.

    writeRegister( 0x5E, 0x04 ); // Set write width to 8 bits and set linear mode.

    const unsigned long memstart( 0x180000 );
    const long xoff( 272 );
    const long yoff( 448 );
    const int width( 32 * glyphWidth() * 2 );
    const int height( glyphHeight() );

    char retval( 0 );
    char s( 0 );

    //char* testBuffer = new char[height * width];
    //char* bufPtr = testBuffer;

    for( int y = 0; y < height; ++y )
    {
        unsigned long addr( memstart + ( ( yoff + y ) * 800 * 2 ) + ( xoff * 2 ) ); // *2, as 16bpp.

        // Set graphics read/write position
        writeRegister(0x5F, addr);
        writeRegister(0x60, addr >> 8);
        writeRegister(0x61, addr >> 16);
        writeRegister(0x62, addr >> 24);

        char c( 0 );
        m_bus.write( BusAddresses::GraphCommand, 0x04 ); // Data port.
        m_bus.read( BusAddresses::GraphData, &c, 1); // Dummy read graphics RAM->RA output buffer
        m_bus.read( BusAddresses::GraphData, &c, 1); // Dummy read PIC input buffer->PMDIN register
        for( int x = 0; x < width; ++x )
        {
            m_timer->usleep( 10 );
            m_bus.read( BusAddresses::GraphData, (char*)&c, 1);
            //*bufPtr++ = c;

            for( int i = 8; i > 0; --i )
            {
                s = ( retval ^ c ) & 0x01;
                retval >>= 1;
                if( s )
                {
                    retval ^= 0xE0;
                }
                c >>= 1;
            }
        }
    }

    writeRegister( 0x03, 0x04 ); // Set text mode.

    writeRegister( 0x5E, 0x01 ); // Set canvas depth to 16bpp and set block mode.

    //hexDump( testBuffer, height * width );

    checkStatus();

    if( m_doLog ) LOG_DEBUG( "LCD: Completed self test" );

    return retval;
}

void RA8873::dumpPerformanceInfo()
{
    if( m_doLog )
    {
        LiteStream stream;
        stream << "Win switches: " << m_winSwitches << "\r\n"
            << "Attr switches: " << m_attrSwitches << "\r\n"
            << "Trans switches: " << m_transSwitches << "\r\n"
            << "Pos switches: " << m_posSwitches;
    }
}

void RA8873::panic( unsigned int address, unsigned int code )
{
    m_doLog = false; // Disable logging to avoid heap use and relying on external dependencies.

    clearRegion( Rectangle( panicWindowX,
                            panicWindowY,
                            panicWindowHeight,
                            panicWindowWidth ) );

    setColours( 0x0A );

    setTransparency( false );

    for( int y = 0; y < sadAgapeHeight; ++y )
    {
        setTextPosition( panicWindowX + ( sadAgapeCol * _glyphWidth ),
                         panicWindowY + ( ( sadAgapeRow + y ) * _glyphHeight ) );
        m_bus.write( BusAddresses::GraphCommand, 0x04 ); // Data port.
        for( int x = 0; x < sadAgapeWidth; ++x )
        {
            writeData( 0x00 );
            writeData( sadAgape[y][x] );
        }
    }

    setTextPosition( panicWindowX + ( panicMessageFirstCol * _glyphWidth ),
                     panicWindowY + ( panicMessageFirstRow * _glyphHeight ) );
    m_bus.write( BusAddresses::GraphCommand, 0x04 ); // Data port.
    for( int i = 0; i < strlen( panicMessage1 ); ++i )
    {
        writeData( 0x00 );
        writeData( panicMessage1[i] );
    }

    setTextPosition( panicWindowX + ( panicMessageFirstCol * _glyphWidth ),
                     panicWindowY + ( ( panicMessageFirstRow + 2 ) * _glyphHeight ) );
    m_bus.write( BusAddresses::GraphCommand, 0x04 ); // Data port.
    for( int i = 0; i < strlen( panicMessage2 ); ++i )
    {
        writeData( 0x00 );
        writeData( panicMessage2[i] );
    }

    setTextPosition( panicWindowX + ( panicMessageFirstCol * _glyphWidth ),
                     panicWindowY + ( ( panicMessageFirstRow + 4 ) * _glyphHeight ) );
    m_bus.write( BusAddresses::GraphCommand, 0x04 ); // Data port.
    for( int i = 0; i < strlen( panicMessage3 ); ++i )
    {
        writeData( 0x00 );
        writeData( panicMessage3[i] );
    }

    setTextPosition( panicWindowX + ( panicMessageFirstCol * _glyphWidth ),
                     panicWindowY + ( ( panicMessageFirstRow + 5 ) * _glyphHeight ) );
    m_bus.write( BusAddresses::GraphCommand, 0x04 ); // Data port.
    writeData( 0x00 );
    writeData( '0' );
    writeData( 0x00 );
    writeData( 'x' );
    char c;
    for( int i( ( sizeof( unsigned int ) * 2 ) - 1 ); i >= 0; --i )
    {
        c = nybbleToHexChar( ( address >> ( i * 4 ) ) & 0xF );
        writeData( 0x00 );
        writeData( c );
    }

    writeData( 0x00 );
    writeData( 0x20 );
    writeData( 0x00 );
    writeData( '0' );
    writeData( 0x00 );
    writeData( 'x' );

    for( int i( ( sizeof( unsigned int ) * 2 ) - 1 ); i >= 0; --i )
    {
        c = nybbleToHexChar( ( code >> ( i * 4 ) ) & 0xF );
        writeData( 0x00 );
        writeData( c );
    }

    while( 1 )
    {
    }
}

void RA8873::writeRegister( int addr, int val )
{
    m_bus.write( BusAddresses::GraphCommand, addr );
    m_bus.write( BusAddresses::GraphData, val );
}

void RA8873::writeData( int data )
{
    waitForWFIFONotFull();
    m_bus.write( BusAddresses::GraphData, data );
}

void RA8873::waitForCoreIdle()
{
    // Wait for core task completed, if any.
    m_timer->reset();

    char status;
    m_bus.read( BusAddresses::GraphCommand, &status, 1 ); // Dummy read
    m_bus.read( BusAddresses::GraphCommand, &status, 1 );
    while( ( status & 0x08 ) && ( m_timer->ms() < timeout ) )
    {
        m_bus.read( BusAddresses::GraphCommand, &status, 1 );
        if( m_doLog )
        {
            LiteStream stream;
            stream << "LCD: Wait for core idle: Status: " << ucharToHex( status );
            LOG_DEBUG( stream.str() );
        }
    }

    if( status & 0x08 )
    {
        if( m_doLog ) LOG_DEBUG( "LCD: Timed out waiting for core task idle" );
    }
}

void RA8873::waitForWFIFOEmpty()
{
    // Wait for write FIFO to be empty.
    m_timer->reset();

    char status;
    m_bus.read( BusAddresses::GraphCommand, &status, 1 ); // Dummy read
    m_bus.read( BusAddresses::GraphCommand, &status, 1 );
    while( !( status & 0x40 ) && ( m_timer->ms() < timeout ) )
    {
        m_bus.read( BusAddresses::GraphCommand, &status, 1 );
        if( m_doLog )
        {
            LiteStream stream;
            stream << "LCD: Wait for write FIFO empty: Status: " << ucharToHex( status );
            LOG_DEBUG( stream.str() );
        }
    }

    if( !( status & 0x40 ) )
    {
        if( m_doLog ) LOG_DEBUG( "LCD: Timed out waiting for write FIFO empty" );
    }
}

void RA8873::waitForWFIFONotFull()
{
    // Wait for write FIFO to be not full.
    m_timer->reset();

    char status;
    m_bus.read( BusAddresses::GraphCommand, &status, 1 ); // Dummy read
    m_bus.read( BusAddresses::GraphCommand, &status, 1 );
    while( ( status & 0x80 ) && ( m_timer->ms() < timeout ) )
    {
        m_bus.read( BusAddresses::GraphCommand, &status, 1 );
        if( m_doLog )
        {
            LiteStream stream;
            stream << "LCD: Wait for write FIFO not full: Status: " << ucharToHex( status );
            LOG_DEBUG( stream.str() );
        }
    }

    if( status & 0x80 )
    {
        if( m_doLog ) LOG_DEBUG( "LCD: Timed out waiting for write FIFO not full" );
    }
}

void RA8873::reset()
{
    // Give an extra delay for the LCD board to power up.
    m_bus.reset(); // Force entire address to be written out, to ensure PLC is in a known state.
    m_bus.write( BusAddresses::GraphReset, 0x00 );
    for( int i = 0; i < 100; ++i ) m_timer->usleep( 100 );

    bool success( false );
    while( !success )
    {
        // Perform reset.
        if( m_doLog ) LOG_DEBUG( "LCD: Asserting reset" );
        m_bus.reset(); // Force entire address to be written out, to ensure PLC is in a known state.
        m_bus.write( BusAddresses::GraphReset, 0x00 );
        m_timer->usleep( 50 ); // Wait min. 256 OSC (@10MHz) - Datasheet 6.2.1.
        m_bus.write( BusAddresses::GraphNotReset, 0x00 );
        if( m_doLog ) LOG_DEBUG( "LCD: Release from reset" );

        // Wait for "inhibit operation" bit to clear - Datasheet 6.2.1.
        m_timer->reset();
        char status;
        m_bus.read( BusAddresses::GraphCommand, &status, 1 ); // Dummy read
        m_bus.read( BusAddresses::GraphCommand, &status, 1 );
        while( ( ( status == 0 ) || ( status & 2 ) ) && m_timer->ms() < timeout )
        {
            m_bus.read( BusAddresses::GraphCommand, &status, 1 );
            if( m_doLog )
            {
                LiteStream stream;
                stream << "LCD: Status: " << ucharToHex( status );
                LOG_DEBUG( stream.str() );
            }
        }

        success = !( ( status == 0 ) || ( status & 2 ) );

        if( !success && m_doLog )
        {
            LOG_DEBUG( "LCD: Reset timed out. Retrying." );
        }
        // Status should now be 0x50.
    }

    if( m_doLog ) LOG_DEBUG( "LCD: Reset complete" );
}

void RA8873::setPLLs()
{
    if( m_doLog ) LOG_DEBUG( "LCD: Setting PLLs" );

    //writeRegister( 0x05, 0x30 ); // SCLK PLL = 60MHz, VCO = 480MHz.
    //writeRegister( 0x06, 0x2f ); // Currently makes jailbars static with small flicker. Could try 90MHz, but seems too much for blue unit panel?
    //writeRegister( 0x07, 0x06 ); // MCLK PLL = 60MHz. Datasheet default.
    //writeRegister( 0x08, 0x2f );
    //writeRegister( 0x09, 0x06 ); // CCLK PLL = 60MHz. Datasheet default.
    //writeRegister( 0x0a, 0x2f );
    //writeRegister( 0x01, 0x80 ); // Set.

    writeRegister( 0x05, 0x30 ); // SCLK PLL
    writeRegister( 0x06, 0x2f );
    writeRegister( 0x07, 0x06 ); // MCLK PLL
    writeRegister( 0x08, 0x2f );
    writeRegister( 0x09, 0x06 ); // CCLK PLL
    writeRegister( 0x0a, 0x2f );
    writeRegister( 0x01, 0x80 ); // Set.

    m_timer->usleep( 100 ); // 1024 OSC, although datasheet also says 30uS...

    // Ensure PLLs enabled.
    m_timer->reset();
    char ccr;
    m_bus.write( BusAddresses::GraphCommand, 0x01 );
    m_bus.read( BusAddresses::GraphData, &ccr, 1 ); // Dummy read
    m_bus.read( BusAddresses::GraphData, &ccr, 1 );
    while( !( ccr & 0x80 ) && ( m_timer->ms() < timeout ) )
    {
        m_bus.read( BusAddresses::GraphData, &ccr, 1 );
        if( m_doLog )
        {
            LiteStream stream;
            stream << "LCD: CCR: " << ucharToHex( ccr );
            LOG_DEBUG( stream.str() );
        }
    }

    if( !( ccr & 0x80 ) )
    {
        if( m_doLog ) LOG_DEBUG( "LCD: Timed out waiting for PLLs to lock" );
    }
    else if( m_doLog ) LOG_DEBUG( "LCD: PLL set complete" );
}

void RA8873::initRAM()
{
    if( m_doLog ) LOG_DEBUG( "LCD: Initialising RAM" );

    writeRegister( 0xe0, 0x28 ); // BFRAR - Default values.
    writeRegister( 0xe1, 0x03 ); // BFRMD - CAS 3
    writeRegister( 0xe2, 0xff ); // Refresh - should be < 1D4 for
    writeRegister( 0xe3, 0x00 ); //           60MHz MCLK.
    writeRegister( 0xe4, 0x01 ); // BFRCR - Default values and start init.

    // Wait for init to complete.
    m_timer->reset();
    char bfrcr;
    m_bus.write( BusAddresses::GraphCommand, 0xE4 );
    m_bus.read( BusAddresses::GraphData, &bfrcr, 1 ); // Dummy read
    m_bus.read( BusAddresses::GraphData, &bfrcr, 1 );
    while( !( bfrcr & 0x01 ) && ( m_timer->ms() < timeout ) )
    {
        m_bus.read( BusAddresses::GraphData, &bfrcr, 1 );
        if( m_doLog )
        {
            LiteStream stream;
            stream << "LCD: BFRCR: " << ucharToHex( bfrcr );
            LOG_DEBUG( stream.str() );
        }
    }

    if( !( bfrcr & 0x01 ) )
    {
        if( m_doLog ) LOG_DEBUG( "LCD: Timed out waiting for RAM initialisation" );
    }
    else if( m_doLog ) LOG_DEBUG( "LCD: RAM initialisation complete" );
}

void RA8873::checkStatus()
{
    // Check for RAM warning status.
    m_bus.write( BusAddresses::GraphCommand, 0x00 );
    m_bus.read( BusAddresses::GraphData, (char*)&m_warn, 1 );
    m_bus.read( BusAddresses::GraphData, (char*)&m_warn, 1 );
    m_warnCount += ( m_warn & 0x80 );
}

void RA8873::setBrightness()
{
    writeRegister( 0x88, m_brightness & 0xFF ); // Set TCMPB0
    writeRegister( 0x89, m_brightness >> 8 );
}

void RA8873::displayCopyright()
{
    setColours( 0x08 );

    setTransparency( false );

    // 63, 64, 65, 66 for text position...
    int x( 80 );
    int y( 448 );
    x += ( ( 80 - copyright.length() ) / 2 ) * 8; // Centre.
    setTextPosition( x, y );
    m_bus.write( BusAddresses::GraphCommand, 0x04 ); // Data port.

    for( int x = 0; x < copyright.length(); ++x )
    {
        writeData( 0x00 );
        writeData( copyright[x] );
    }
}

void RA8873::clearRegion( const Rectangle& clearRect, unsigned char colour )
{
    Rectangle clipRect( clearRect.clipTo( m_screenRect ) );

    if( ( clipRect.height() <= 0 ) ||
        ( clipRect.width() <= 0 ) )
    {
        return;
    }

    if( m_doLog )
    {
        LiteStream stream;
        stream << "LCD: Clearing region"
               << " x: " << clipRect.originX()
               << " y: " << clipRect.originY()
               << " height: " << clipRect.height()
               << " width: " << clipRect.width()
               << " colour: " << colour;
        LOG_DEBUG( stream.str() );
    }

    waitForCoreIdle();

    // Clear with block transfer engine.
    writeRegister( 0xA7, 0x00 ); // Set desination memory start address
    writeRegister( 0xA8, 0x00 ); // to 0x00180000 (first block).
    writeRegister( 0xA9, 0x18 );
    writeRegister( 0xAA, 0x00 );

    writeRegister( 0xAB, 0x20 ); // Set destination image width to 800px.
    writeRegister( 0xAC, 0x03 );

    writeRegister( 0x92, 0x25 ); // Set destination color depth to 16bpp. // Was 0x01

    writeRegister( 0xAD, clipRect.originX() ); // Set destination window upper left X coords.
    writeRegister( 0xAE, clipRect.originX() >> 8 );
    writeRegister( 0xAF, clipRect.originY() ); // Set destination window upper left Y coords.
    writeRegister( 0xB0, clipRect.originY() >> 8 );

    writeRegister( 0xB1, clipRect.width() ); // Set BTE window width.
    writeRegister( 0xB2, clipRect.width() >> 8 );
    
    writeRegister( 0xB3, clipRect.height() ); // Set BTE window height.
    writeRegister( 0xB4, clipRect.height() >> 8 );

    // D2-D4
    uint32_t fgColour( ARGBColours[ colour & 0x0F ] );
    writeRegister( 0xD2, fgColour );       // R // Set foreground colour.
    writeRegister( 0xD3, fgColour >> 8 );  // G
    writeRegister( 0xD4, fgColour >> 16 ); // B
    m_attribute = ( m_attribute & 0xF0 ) + fgColour;

    writeRegister( 0x91, 0xFC ); // ROP code = whiteness, operation = solid fill.

    writeRegister( 0x90, 0x10 ); // Enable BTE.

    // Wait for BTE idle.
    m_timer->reset();
    char btectrl0;
    m_bus.write( BusAddresses::GraphCommand, 0x90 );
    m_bus.read( BusAddresses::GraphData, &btectrl0, 1 ); // Dummy read
    m_bus.read( BusAddresses::GraphData, &btectrl0, 1 );
    while( ( btectrl0 & 0x10 ) && ( m_timer->ms() < timeout ) )
    {
        m_bus.read( BusAddresses::GraphData, &btectrl0, 1 );
        if( m_doLog )
        {
            LiteStream stream;
            stream << "LCD: BTECTRL0: " << ucharToHex( btectrl0 );
            LOG_DEBUG( stream.str() );
        }
    }

    if( ( btectrl0 & 0x10 ) )
    {
        if( m_doLog ) LOG_DEBUG( "LCD: Timed out waiting for BTE operation to complete" );
    }
    else if( m_doLog ) LOG_DEBUG( "LCD: Clear region complete" );
}

void RA8873::setColours( unsigned char attribute )
{
    waitForCoreIdle();

    uint32_t fgColour( ARGBColours[ attribute & 0x0F ] );
    uint32_t bgColour( ARGBColours[ attribute >> 4 ] );
    writeRegister( 0xD2, fgColour );
    writeRegister( 0xD3, fgColour >> 8 );
    writeRegister( 0xD4, fgColour >> 16 );

    writeRegister( 0xD5, bgColour );
    writeRegister( 0xD6, bgColour >> 8 );
    writeRegister( 0xD7, bgColour >> 16 );
}

void RA8873::setTransparency( bool transparency )
{
    waitForCoreIdle();

    if( transparency )
    {
        writeRegister( 0xCD, 0x40 ); // Chroma key.
    }
    else
    {
        writeRegister( 0xCD, 0x00 );
    }
}

void RA8873::setTextPosition( int x, int y )
{
    waitForCoreIdle();

    writeRegister( 0x63, x );
    writeRegister( 0x64, x >> 8 );
    writeRegister( 0x65, y );
    writeRegister( 0x66, y >> 8 );
}

void RA8873::loadCGRAM()
{
    if( m_doLog ) LOG_DEBUG( "LCD: Loading CGRAM" );

    waitForCoreIdle();

    writeRegister( 0x03, 0x00 ); // Set graphics mode.

    writeRegister( 0x5E, 0x04 ); // Set write width to 8 bits and set linear mode.

    writeRegister(0x5F, 0x00); // Set graphics read/write position
    writeRegister(0x60, 0x00); // to 0x00380000 (second block).
    writeRegister(0x61, 0x38);
    writeRegister(0x62, 0x00);

    m_bus.write( BusAddresses::GraphCommand, 0x04 ); // Data port.
    for( int i = 0; i < 256 + 38; ++i )
     {
        for( int j = 0; j < 16; ++j )
        {
            m_timer->usleep( 10 );
            if( i < 256 )
            {
                writeData( vgaGlyphs[(i*16)+j] );
            }
            else
            {
                writeData( avatarGlyphs[((i-256)*16)+j] );
            }
        }
    }

    waitForWFIFOEmpty();

    writeRegister(0xdb, 0x00); // Set CGRAM start address
    writeRegister(0xdc, 0x00); // to 0x00380000 (second block).
    writeRegister(0xdd, 0x38);
    writeRegister(0xde, 0x00);

    writeRegister( 0xcc, 0x80 ); // Set character source to user-defined, height 16px.

    writeRegister( 0x03, 0x04 ); // Set text mode.

    writeRegister( 0x5E, 0x01 ); // Set canvas depth to 16bpp and set block mode.

    if( m_doLog ) LOG_DEBUG( "LCD: CGRAM load complete" ); 
}

} // namespace GraphicsDrivers

} // namespace agape
