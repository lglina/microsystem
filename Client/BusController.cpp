#include "BusAddresses.h"
#include "BusController.h"

#include "cpu.h"

#include <xc.h>

#if defined(__32MX170F256B__)
#define NOT_ALL_SET LATBSET = 0x0010
#define NOT_ALL_CLR LATBCLR = 0x0010
#elif defined(__32MX470F512H__)
#define NOT_ALL_SET LATBSET = 0x8000
#define NOT_ALL_CLR LATBCLR = 0x8000
#define NOT_ALH_SET LATBSET = 0x4000
#define NOT_ALH_CLR LATBCLR = 0x4000
#elif defined(__32MZ2048EFG064__)
#define NOT_ALL_SET LATBSET = 0x8000
#define NOT_ALL_CLR LATBCLR = 0x8000
#define NOT_ALH_SET LATBSET = 0x4000
#define NOT_ALH_CLR LATBCLR = 0x4000
#endif

#define DIRECT_CS
#define USING_ALH

#ifdef DIRECT_CS
#if defined(__32MX470F512H__)
// RB2
#define NOT_LCD_RESET_SET LATBSET = 0x0004
#define NOT_LCD_RESET_CLR LATBCLR = 0x0004
// RB15
#define NOT_LCD_CS_SET LATBSET = 0x8000
#define NOT_LCD_CS_CLR LATBCLR = 0x8000
// RB14
#define LCD_RS_SET LATBSET = 0x4000
#define LCD_RS_CLR LATBCLR = 0x4000
#elif defined(__32MZ2048EFG064__)
// RG8
#define NOT_LCD_RESET_SET LATGSET = 0x0100
#define NOT_LCD_RESET_CLR LATGCLR = 0x0100
// RG7
#define NOT_LCD_CS_SET LATGSET = 0x0080
#define NOT_LCD_CS_CLR LATGCLR = 0x0080
// RG6
#define LCD_RS_SET LATGSET = 0x0040
#define LCD_RS_CLR LATGCLR = 0x0040
// RB2
#define NOT_CS_X1_SET LATBSET = 0x0004
#define NOT_CS_X1_CLR LATBCLR = 0x0004
// RB3
#define NOT_CS_X2_SET LATBSET = 0x0008
#define NOT_CS_X2_CLR LATBCLR = 0x0008
// RB4
#define NOT_CS_FLASH_SET LATBSET = 0x0010
#define NOT_CS_FLASH_CLR LATBCLR = 0x0010
// RB5
#define NOT_CS_ESTELLE_SET LATBSET = 0x0020
#define NOT_CS_ESTELLE_CLR LATBCLR = 0x0020
#endif
#endif

namespace Agape
{

using namespace BusAddresses;

BusController::BusController() :
  m_address( 0 )
{
    PMCONbits.ON = 0;
    PMCONbits.PTWREN = 1;
    PMCONbits.PTRDEN = 1;
#ifdef USING_ALH
    PMCONbits.ADRMUX = 2;
    PMAENbits.PTEN = 3;
#else
    PMCONbits.ADRMUX = 0x01;
    PMAENbits.PTEN = 1;
#endif
    PMMODEbits.MODE = 2;

    NOT_ALL_SET; // Deassert /ALL
#ifdef USING_ALH
    NOT_ALH_SET; // Deassert /ALH
#endif

    PMMODEbits.WAITM = ( PBCLK_FREQ / 8000000 ) - 1;
    PMMODEbits.WAITB = ( PBCLK_FREQ / 8000000 ) - 1;
    PMMODEbits.WAITE = ( PBCLK_FREQ / 8000000 ) - 1;

    PMCONbits.ON = 1;
}

int BusController::read( int address, char* data, int len )
{
    setAddress( address );

    // Note: It's up to the caller to do a dummy read first, or to
    // expect the first returned byte to be old data.
    for( int i = 0; i < len; ++i )
    {
        while( PMMODEbits.BUSY == 1 ) {}
        *data++ = PMDIN;
    }

    return len;
}

int BusController::write( int address, const char* data, int len )
{
    setAddress( address );

    for( int i = 0; i < len; ++i )
    {
        while( PMMODEbits.BUSY == 1 ) {}
        PMDIN = *data++;
    }
    return len;
}

bool BusController::error()
{
    return false;
}

void BusController::write( int address, int data )
{
    setAddress( address );
    while( PMMODEbits.BUSY == 1 ) {}
    PMDIN = data;
}

void BusController::reset()
{
    m_address = 0;
}

void BusController::setAddress( int address )
{
    if( address == m_address )
    {
        // Address unchanged.
        // Don't write address.
        // Pin off first.
        PMAENbits.PTEN = 0;
        PMCONbits.ADRMUX = 0x00;
    }
    else if( ( ( m_address != 0 ) &&
             ( address & 0xFF00 ) == 0 ) &&
             ( ( m_address & 0xFF00 ) == 0 ) )
    {
        // Previous and current addresses both low.
        // Write only low address bits.
        if( PMCONbits.ADRMUX == 0x00 )
        {
            // Pin on last.
            PMCONbits.ADRMUX = 0x01;
            PMAENbits.PTEN = 1;
        }
        else if( PMCONbits.ADRMUX == 0x02 )
        {
            // Pin off first.
            PMAENbits.PTEN = 1;
            PMCONbits.ADRMUX = 0x01;
        }
    }
    else
    {
#ifdef USING_ALH
        // Write all address bits.
        // Pin on last.
        PMCONbits.ADRMUX = 0x02;
        PMAENbits.PTEN = 3;
#else
        PMCONbits.ADRMUX = 0x01;
        PMAENbits.PTEN = 1;
#endif
    }

    PMADDR = address;
    m_address = address;

#ifdef DIRECT_CS
    // We don't have external logic driving the LCD and CS signals so set
    // them now, if an LCD or CS address is selected.

    while( PMMODEbits.BUSY == 1 ) {}

    if( address == CSEstelle )
    {
        NOT_CS_FLASH_SET;
        NOT_CS_X1_SET;
        NOT_CS_X2_SET;
        NOT_LCD_CS_SET;
        NOT_CS_ESTELLE_CLR;
    }
    else if( address == CSFlash )
    {
        NOT_CS_ESTELLE_SET;
        NOT_CS_X1_SET;
        NOT_CS_X2_SET;
        NOT_LCD_CS_SET;
        NOT_CS_FLASH_CLR;
    }
    else if( address == CSX1 )
    {
        NOT_CS_ESTELLE_SET;
        NOT_CS_FLASH_SET;
        NOT_CS_X2_SET;
        NOT_LCD_CS_SET;
        NOT_CS_X1_CLR;
    }
    else if( address == CSX2 )
    {
        NOT_CS_ESTELLE_SET;
        NOT_CS_FLASH_SET;
        NOT_CS_X1_SET;
        NOT_LCD_CS_SET;
        NOT_CS_X2_CLR;
    }
    else if( address == GraphCommand )
    {
        NOT_CS_ESTELLE_SET;
        NOT_CS_FLASH_SET;
        NOT_CS_X1_SET;
        NOT_CS_X2_SET;
        NOT_LCD_CS_CLR;
        LCD_RS_CLR;
    }
    else if( address == GraphData )
    {
        NOT_CS_ESTELLE_SET;
        NOT_CS_FLASH_SET;
        NOT_CS_X1_SET;
        NOT_CS_X2_SET;
        NOT_LCD_CS_CLR;
        LCD_RS_SET;
    }
    else if( address == GraphNotReset )
    {
        NOT_CS_ESTELLE_SET;
        NOT_CS_FLASH_SET;
        NOT_CS_X1_SET;
        NOT_CS_X2_SET;
        NOT_LCD_CS_SET;
        NOT_LCD_RESET_SET;
    }
    else if( address == GraphReset )
    {
        NOT_CS_ESTELLE_SET;
        NOT_CS_FLASH_SET;
        NOT_CS_X1_SET;
        NOT_CS_X2_SET;
        NOT_LCD_CS_SET;
        NOT_LCD_RESET_CLR;
    }
    else
    {
        NOT_CS_ESTELLE_SET;
        NOT_CS_FLASH_SET;
        NOT_CS_X1_SET;
        NOT_CS_X2_SET;
        NOT_LCD_CS_SET;
    }
#endif

/////////////////////
#ifdef false
    // We don't have external logic driving the LCD signals so set
    // them now, if an LCD address is selected.

    while( PMMODEbits.BUSY == 1 ) {}

    if( address == BusAddresses::GraphCommand )
    {
        NOT_LCD_CS_CLR;
        LCD_RS_CLR;
    }
    else if( address == BusAddresses::GraphData )
    {
        NOT_LCD_CS_CLR;
        LCD_RS_SET;
    }
    else if( address == BusAddresses::GraphReset )
    {
        NOT_LCD_CS_SET;
        NOT_LCD_RESET_CLR;
    }
    else
    {
        NOT_LCD_CS_SET;
        NOT_LCD_RESET_SET;
    }
#endif
}

} // namespace Agape
