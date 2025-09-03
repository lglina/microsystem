#pragma config JTAGEN = OFF
#pragma config FWDTEN = OFF, SOSCSEL = ON, SOSCEN = OFF, FNOSC = 0
#pragma config FCKSM = 1, PLLSRC = 1
// FNOSC = 0 selects Fast RC oscillator (FRC).

#include "Loggers/Logger.h"
#include "Loggers/SerialLogger.h"
#include "Estelle.h"
#include "EstelleBuilder.h"
#include "InterruptHandler.h"
#include "PICSerial.h"

#include <xc.h>

using namespace Agape;

int main( int argc, char** argv )
{
    INTCONSET = _INTCON_MVEC_MASK;

#ifdef __32MM0064GPL028__
    TRISA = 0x001B;
    TRISB = 0xC07F; // FIXME: /RESET and /PWRON output disabled!
    //TRISB = 0x407F;
    TRISC = 0x0200;
    ANSELA = 0x0003;
    ANSELB = 0x0000;
    CNPUB = 0x007F;

    // SPI assignments...
    // RA2 = RP3 = Master SDI
    // RA3 = RP4 = Master SDO
    // RA4 = RP6 = Master SCK
    RPINR11bits.SDI2R = 4; // SDI2
    RPINR11bits.SCK2INR = 6; // SCK2
    RPOR0bits.RP3R = 3; // SDO2
    RPINR11bits.SS2INR = 19; // SS2

    //PORTBSET = 0x8000; // Power off.
#endif

#ifdef __32MM0256GPM064__
    TRISA = 0xFFFF; // Keyboard - all inputs initially.
    TRISB = 0xE90C; // IBATT, VBATT, /CHG, /EXTPWR, /CARD_INSERTED, AMB_SENSOR, LID_SENSOR inputs.
    TRISC = 0x0041; // ESTELLE_SDI, ENTROPY inputs.
    TRISD = 0x0014; // SDO (from master), /CS_ESTELLE inputs.

    LATA = 0x0000;
    LATB = 0x04E0; // Enable sensors, disable flight, disable power, hold reset, de-assert alert.
    LATC = 0x0000;
    LATD = 0x0000;

    ANSELA = 0x0000;
    ANSELB = 0xC00C;
    ANSELC = 0x0000;
    
    CNPUA = 0x00FF;
    CNPUB = 0x2900;

    // SPI assignments...
    
    // SPI1:
    // RD0 = SDO, to master SDI.
    // RD4 = SDI, from master SDO.
    // RC13 = Master SCK
    // RD2 = Slave Select
    
    // SPI2:
    RPINR11bits.SDI2R = 0x17; // RP23/RC6
    RPOR4bits.RP18R = 0x09; // SCK2OUT
    RPOR4bits.RP20R = 0x08; // SDO2

    RPOR3bits.RP14R = 0x04; // U2TX

    IPC4bits.T2IP = 1;
    IPC14bits.U2TXIP = 1;
    IPC14bits.U2RXIP = 1;
    IPC10bits.SPI1TXIP = 1;
    IPC10bits.SPI1RXIP = 1;

    PICSerial serial( 2, 115200, 256, 8 );
    InterruptDispatcher::instance()->registerHandler( InterruptDispatcher::UART2Tx, &serial );
    InterruptDispatcher::instance()->registerHandler( InterruptDispatcher::UART2Rx, &serial );
    Logger::setInstance( new Loggers::Serial( serial ) );
#endif

    /*
    SPLLCONbits.PLLMULT = 0;
    SPLLCONbits.PLLODIV = 0;

    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    OSCCONbits.NOSC = 1;
    OSCCONSET = 1;

    SYSKEY = 0x00000000;
    while( OSCCONbits.OSWEN );
    */

    __builtin_enable_interrupts();

    LOG_DEBUG( "Estelle starting" );
    LOG_DEBUG( "(C) Lauren Glina 2019-2025" );

    EstelleBuilder estelleBuilder;
    Estelle* estelle( nullptr );

    while( 1 )
    {
        if( estelle == nullptr )
        {
            LOG_DEBUG( "Building" );
            estelle = estelleBuilder.build( serial );
            LOG_DEBUG( "Done" );
        }
        estelle->run();
    }
}
