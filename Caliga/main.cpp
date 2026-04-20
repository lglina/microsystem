// The config words should be the same as those assumed by the application.
// See the application main.cpp.
#if defined(__32MZ2048EFG064__)
#pragma config TRCEN = OFF, DEBUG = OFF // DEVCFG0
#pragma config FDMTEN = OFF, FWDTEN = OFF, FSOSCEN = OFF, FNOSC = SPLL // DEVCFG1
#pragma config FPLLODIV = DIV_2, FPLLMULT = MUL_16, FPLLIDIV = DIV_2 // DEVCFG2
#pragma config FUSBIDIO = OFF, IOL1WAY = OFF // DEVCFG3
#pragma config BOOTISA=MICROMIPS
#endif

#include "AssetLoaders/CaligaAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "GraphicsDrivers/RA8873.h"
#include "Loggers/Logger.h"
#include "Loggers/SerialLogger.h"
#include "Memories/SPIFlash.h"
#include "Timers/Factories/PIC32PrecisionTimerFactory.h"
#include "Timers/Factories/PIC32TimerFactory.h"
#include "Timers/PIC32AbsoluteTimer.h"
#include "Timers/PIC32PrecisionTimer.h"
#include "Timers/Timer.h"
#include "Utils/StrToHex.h"
#include "World/WorldCoordinates.h"
#include "ANSITerminal.h"
#include "BusController.h"
#include "InterruptHandler.h"
#include "PICSerial.h"
#include "SPIController.h"
#include "WindowManager.h"

#include "crc/checksum.h"

#include <xc.h>

using namespace Agape;

#define APP_RESET_ADDRESS       0xBD000000u

#define ROW_SIZE                2048

#define NVMOP_PROGRAM_ERASE     0x07
#define NVMOP_ROW_PROGRAM       0x03

// The flash programming peripheral is on the system bus and not interactive
// with the CPU d-cache, therefore use the coherent attribute to ensure this
// buffer is not placed in the cacheable virtual memory region.
char rowbuff[ROW_SIZE] __attribute__ ((coherent));

void die( const String& message, Terminal& terminal )
{
    terminal.consumeNext( 21, 19, Terminal::colBrightRed );
    terminal.consumeString( "Fatal error. Hold Esc to restart." );
    terminal.consumeNext( 22, 19 );
    terminal.consumeString( message );
    
    while( 1 ) {}
}

void doFlashOp( char op, Terminal& terminal )
{
    // Per PIC32 FRM s. 52.
    NVMCONbits.NVMOP = op;
    NVMCONbits.WREN = 1;
    __builtin_disable_interrupts();
    NVMKEY = 0x00;
    NVMKEY = 0xAA996655;
    NVMKEY = 0x556699AA;
    NVMCONSET = 1 << 15;
    __builtin_enable_interrupts();
    while( NVMCONbits.WR );
    NVMCONbits.WREN = 0;

    if( NVMCON & 0x3000 )
    {
        die( "Writing PF", terminal );
    }
}

void jumpToApp()
{
    __builtin_disable_interrupts();

    void (*fptr)(void);
    fptr = (void (*)(void))APP_RESET_ADDRESS;
    fptr();
}

int main( int argc, char** argv )
{
    INTCONSET = _INTCON_MVEC_MASK;

    OSCCONbits.COSC = 0x01; // Use SPLL

    // These are just copied from the application code. We could cut this down
    // to a subset of just the peripherals the bootloader needs...
#ifdef __32MZ2048EFG064__
    TRISB = 0x0000; // All outputs.
    TRISC = 0x6000; // /DCD and /RI inputs.
    TRISD = 0x0205; // /INT, SDI and /CTS inputs.
    TRISE = 0x0000; // PMP peripheral will control D7-D0 (RE7~RE0).
    TRISF = 0x0022; // RX and RX2 inputs.
    TRISG = 0x0000; // All outputs

    LATB = 0xC07C; // /CS.X1, /CS.X2, /CS.FLASH, /CS.ESTELLE, /RTS, /ALH, /ALL de-asserted.
    LATC = 0x0000;
    LATD = 0x0030; // /WR and /RD de-asserted.
    LATE = 0x0000;
    LATF = 0x0000;
    LATG = 0x0080; // /CS.LCD de-asserted. /LCD_RESET asserted.

    ANSELB = 0x0000;
    ANSELE = 0x0000;
    ANSELG = 0x0000;

    CNPUC = 0x6000; // /DCD and /RI pulled up.
    CNPUD = 0x0201; // /INT and /CTS pulled up.

    U1CTSRbits.U1CTSR = 0x00; // RPD9
    RPB6Rbits.RPB6R = 0x01; // /U1RTS
    SDI1Rbits.SDI1R = 0x00; // RPD2
    RPD3Rbits.RPD3R = 0x05; // SDO1
    U1RXRbits.U1RXR = 0x04; // RPF1
    RPF0Rbits.RPF0R = 0x01; // U1TX
    U3RXRbits.U3RXR = 0x02; // RPF5
    RPF4Rbits.RPF4R = 0X01; // U3TX
    RPD11Rbits.RPD11R = 0x03; // U5TX

    IPC28bits.U1TXIP = 1;
    IPC28bits.U1RXIP = 1;
    IPC39bits.U3TXIP = 2; // Higher prio for MIDI.
    IPC39bits.U3RXIP = 2;
    IPC45bits.U5TXIP = 1;
    IPC45bits.U5RXIP = 1;
    IPC1bits.T1IP = 1;
    IPC2bits.T2IP = 1;
    IPC27bits.SPI1TXIP = 1;
    IPC27bits.SPI1RXIP = 1;
#endif

    __builtin_enable_interrupts();

#if defined(__32MZ2048EFG064__)
    PICSerial* debugSerial = new PICSerial( 5, 115200, 768, 16 );
    Logger::setInstance( new Loggers::Serial( *debugSerial ) );
#endif
    LOG_DEBUG( "Caliga" );

    // Set up SPI for external flash.
    BusController bus;
    Timers::PIC32Absolute* absoluteTimer = new Timers::PIC32Absolute( &bus );
    Timers::Factories::PIC32TimerFactory timerFactory;

    SPIController spiController( 1,
                                 4000000, // Hz
                                 true, // true = master
                                 timerFactory );
    Memories::SPIFlash updateMemory( spiController,
                                     bus,
                                     0x200000,
                                     0x100,
                                     0x1000,
                                     0x10000,
                                     0x600000 ); // <- Offset at which the application will have written the update.

    // Read update CRC to see if there's an update pending.
    uint16_t updateCRC( 0x0 );
    updateMemory.read( 0x1FFFFE, (char*)&updateCRC, 2 );

    if( updateCRC == 0xFFFFu ) // Update memory blank.
    {
#if defined(__32MZ2048EFG064__)
        Logger::setInstance( nullptr );
        debugSerial->flushOutput();
        delete( debugSerial );
#endif
        delete( absoluteTimer );
        jumpToApp();
    }

    // Set up graphics and draw update screen.
    Timers::Factories::PIC32PrecisionTimerFactory precisionTimerFactory;
    GraphicsDrivers::RA8873 graphicsDriver( bus, precisionTimerFactory );
    InterruptDispatcher::s_graphicsDriver = &graphicsDriver;

    WindowManager windowManager( graphicsDriver );
    Timer* timer( timerFactory.makeTimer() );
    ANSITerminal terminal( 80,
                           25,
                           "main",
                           graphicsDriver,
                           *timer );

    GraphicsDriver::Window window;
    window.m_name = "main";
    window.m_rect = Rectangle( 80, 48, 400, 640 );
    window.m_visible = true;
    WindowManager::TerminalWindow terminalWindow;
    terminalWindow.m_terminal = &terminal;
    terminalWindow.m_window = graphicsDriver.createWindow( window );
    windowManager.createTerminalWindow( terminalWindow );

    AssetLoaders::Caliga assetLoader( World::Coordinates(), "updatescreen" );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        terminal.consumeNext( 4, 19 );
        terminal.consumeAsset( ansiFile, 0, ansiFile.dataSize(), ansiFile.width(), 19, Terminal::noMaxRow );
    }

    terminal.consumeNext( 16, 20 );

    // Verify update integrity - recalculate the update CRC and
    // see if it matches.
    LOG_DEBUG( "Verifying CRC" );
    uint16_t verifyCRC( 0xFFFF );
    char page[0x100];
    int didRead( 0 );
    for( int offset = 0x0; offset < 0x1FFFFE; offset += didRead )
    {
        // Update progress bar.
        if( ( offset == 0 ) ||
            ( ( offset % 0x80000 ) == 0 ) )
        {
            terminal.consumeString( "\xdb" );
        }

        // Read 256B and update CRC.
        int toRead( ( offset + 0x100 >= 0x1FFFFE ) ? ( 0x1FFFFE - offset ) : 0x100 );
        didRead = updateMemory.read( offset, page, toRead );
        for( int i = 0; i < didRead; ++i )
        {
            verifyCRC = update_crc_ccitt( verifyCRC, page[i] );
        }
    }

    LOG_DEBUG( uintToHex( updateCRC ) );
    LOG_DEBUG( uintToHex( verifyCRC ) );

    if( verifyCRC != updateCRC )
    {
        // Erase last sector so we don't attempt to program again.
        updateMemory.erase( updateMemory.size() - updateMemory.sectorSize(), updateMemory.sectorSize() );
        die( "UF CRC mismatch", terminal );
    }

    // Erase the program flash
    LOG_DEBUG( "Erasing PF" );
    doFlashOp( NVMOP_PROGRAM_ERASE, terminal );

    // Write the program flash
    for( unsigned int offset = 0x0; offset < 0x200000u; offset += ROW_SIZE )
    {
        if( ( offset == 0 ) ||
            ( ( offset % 0x10000u ) == 0 ) )
        {
            terminal.consumeString( "\xdb" );
        }

        // SPIController has an internal 260 byte ring buffer for reads,
        // so fill our 2kiB row buffer with eight 256 byte reads.
        for( int numRead = 0; numRead < ROW_SIZE; numRead += 0x100 )
        {
            if( updateMemory.read( offset + numRead, (char*)rowbuff + numRead, 0x100 ) != 0x100 )
            {
                die( "Reading UF", terminal );
            }
        }

        bool isEmpty( true );
        for( int numRead = 0; numRead < ROW_SIZE; ++numRead )
        {
            if( rowbuff[numRead] != '\xff' )
            {
                isEmpty = false;
                break;
            }
        }

        if( isEmpty )
        {
            // Don't bother writing empty rows.
            continue;
        }

        // The flash peripheral operates directly on the system bus and does
        // not interact with the MMU, so both addresses here must be in
        // physical address space, not virtual address space.
        unsigned int addr( 0x1D000000u + offset );
        NVMADDR = addr;
        NVMSRCADDR = (unsigned int)( (int)rowbuff & 0x1FFFFFFFu );
        LOG_DEBUG( uintToHex( (unsigned int)NVMSRCADDR )
                   + " >> "
                   + uintToHex( (unsigned int)NVMADDR) );
        doFlashOp( NVMOP_ROW_PROGRAM, terminal );
    }

    LOG_DEBUG( "Verifying CRC" );
    uint16_t pmemcrc( 0xFFFF );
    for( unsigned int offset = 0x0; offset < 0x1FFFFEu; ++offset )
    {
        if( ( offset == 0 ) ||
            ( ( offset % 0x100000 ) == 0 ) )
        {
            terminal.consumeString( "\xdb" );
        }

        pmemcrc = update_crc_ccitt( pmemcrc, (unsigned char)*( (unsigned char*)( 0xBD000000u + offset ) ) );
    }

    LOG_DEBUG( uintToHex( updateCRC ) );
    LOG_DEBUG( uintToHex( pmemcrc ) );

    if( pmemcrc != updateCRC )
    {
        die( "PF CRC mismatch", terminal );
    }

    // Erase last sector so we don't attempt to program again.
    LOG_DEBUG( "Erasing UF" );
    updateMemory.erase( updateMemory.size() - updateMemory.sectorSize(), updateMemory.sectorSize() );

#if defined(__32MZ2048EFG064__)
    Logger::setInstance( nullptr );
    debugSerial->flushOutput();
    delete( debugSerial );
#endif
    delete( absoluteTimer );
    jumpToApp();
}
