#if defined(__32MX170F256B__) || defined(__32MX470F512H__)
#pragma config JTAGEN = OFF
#pragma config FWDTEN = OFF, FPBDIV = DIV_1, FSOSCEN = OFF, FNOSC = FRCPLL // FNOSC to FRC for PLL off, FRCPLL for PLL on.
#pragma config FPLLIDIV = DIV_2, FPLLMUL = MUL_16, FPLLODIV = DIV_2 // 32 MHz
#endif

#if defined(__32MZ2048EFG064__)
#pragma config TRCEN = OFF, DEBUG = OFF // DEVCFG0
#pragma config FDMTEN = OFF, FWDTEN = OFF, FSOSCEN = OFF, FNOSC = SPLL // DEVCFG1
#pragma config FPLLODIV = DIV_2, FPLLMULT = MUL_16, FPLLIDIV = DIV_2 // DEVCFG2
#pragma config FUSBIDIO = OFF, IOL1WAY = OFF // DEVCFG3
#endif

#include "Timers/PIC32AbsoluteTimer.h"
#include "BoopieOfflineClientBuilder.h"
#include "BoopieOnlineClientBuilder.h"
#include "Chooser.h"
#include "ClientBuilder.h"
#include "Client.h"
#include "Loggers/Logger.h"
#include "Loggers/SerialLogger.h"
#include "Platforms/BoopiePlatform.h"
#include "PICSerial.h"
#include "InterruptHandler.h"
#include "Collections.h"
#include "String.h"

#include <xc.h>

using namespace Agape;

int main( int argc, char** argv )
{
    INTCONSET = _INTCON_MVEC_MASK;

    OSCCONbits.COSC = 0x01;

#ifdef __32MX170F256B__
    TRISA = 0x0007;
    TRISB = 0x0004;
    ANSELA = 0x0000;
    ANSELB = 0x0000;

    RPB3Rbits.RPB3R = 0x01; // U1TX
    U1RXRbits.U1RXR = 0x04; // RPB2
    RPA3Rbits.RPA3R = 0x02; // U2TX
    U2RXRbits.U2RXR = 0x00; // RPA1
    RPA4Rbits.RPA4R = 0x04; // SDO2
    SDI2Rbits.SDI2R = 0x00; // RPA2

    IPC8bits.U1IP = 1;
    IPC9bits.U2IP = 2; // For MIDI playback.
    IPC1bits.T1IP = 1;
    IPC2bits.T2IP = 1;
#endif

#ifdef __32MX470F512H__
    TRISB = 0x0000;
    TRISC = 0x0000; // All unused.
    TRISD = 0x02C9; // /CTS, /RI, /DCD, SDI, /INT inputs.
    TRISE = 0x0000; // PMP peripheral will control D7-D0.
    TRISF = 0x000A; // USBID, RX inputs.
    TRISG = 0x0000; // All unused.
    
    LATB = 0xC000; // /ALH, /ALL, CLAIRE_PWDN de-asserted.
    LATC = 0x0000;
    LATD = 0x0130; // /RTS, /RD, /WR de-asserted.
    LATE = 0x0000;
    LATF = 0x0000;
    LATG = 0x0000;

    ANSELB = 0x0000;
    ANSELD = 0x0000;
    ANSELE = 0x0000;
    ANSELG = 0x0000;

    CNPUD = 0x0001; // Pull up /INT.

    RPF0Rbits.RPF0R = 0x03; // U1TX
    U1RXRbits.U1RXR = 0x04; // RPF1
    RPD8Rbits.RPD8R = 0x03; // /U1RTS
    U1CTSRbits.U1CTSR = 0x00; // RPD9
    RPF4Rbits.RPF4R = 0x01; // U3TX
    //U3RXRbits.U3RXR = 0x02; // RPF5
    RPF5Rbits.RPF5R = 0x01; // U2TX
    RPD1Rbits.RPD1R = 0x08; // SDO1
    SDI1Rbits.SDI1R = 0x00; // RPD3
    // SCK1 = RD2

    IPC7bits.U1IP = 1;
    IPC9bits.U2IP = 2;
    IPC9bits.U3IP = 2;
    IPC1bits.T1IP = 1;
    IPC2bits.T2IP = 1;
    IPC7bits.SPI1IP = 1;
#endif

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

    Platforms::Boopie::setHeapStart();

#if defined(__32MX470F512H__)
    PICSerial debugSerial( 2, 115200, 768, 16 );
    InterruptDispatcher::instance()->registerHandler( InterruptDispatcher::UART2, &debugSerial );
    Logger::setInstance( new Loggers::Serial( debugSerial ) );
#elif defined(__32MZ2048EFG064__)
    PICSerial debugSerial( 5, 115200, 768, 16 );
    InterruptDispatcher::instance()->registerHandler( InterruptDispatcher::UART5Tx, &debugSerial );
    InterruptDispatcher::instance()->registerHandler( InterruptDispatcher::UART5Rx, &debugSerial );
    Logger::setInstance( new Loggers::Serial( debugSerial ) );
#endif
    LOG_DEBUG( "Boopie starting" );
    LOG_DEBUG( "(C) Lauren Glina 2019-2025" );

    String initialChoice( "Online" );
    Chooser chooser;
    LOG_DEBUG( "Creating builders" );
    chooser.addChoice( new ClientBuilders::BoopieOnline( debugSerial ), "Online" );
    //chooser.addChoice( new ClientBuilders::BoopieOffline, "Offline" );
    LOG_DEBUG( "Setting initial builder " + initialChoice );
    if( chooser.setCurrentChoice( initialChoice ) )
    {
        LOG_DEBUG( "Initial builder set" );
    }
    else
    {
        LOG_DEBUG( "Unable to set initial builder!" );
        while( 1 ) {}
    }

    ClientBuilder* currentBuilder( nullptr );
    Client* client( nullptr );

    while( 1 )
    {
        if( chooser.changed() )
        {
            LOG_DEBUG( "Builder changed. Un-building client." );
            currentBuilder->unbuild();
            client = nullptr;
            chooser.resetChanged();
        }

        if( !client )
        {
            LOG_DEBUG( "Building new client." );
            currentBuilder = chooser.currentChoice();
            client = currentBuilder->build( chooser );
        }

        client->run();
    }
}
