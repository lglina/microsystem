#ifndef AGAPE_GRAPHICS_DRIVERS_RA8873_H
#define AGAPE_GRAPHICS_DRIVERS_RA8873_H

#include "GraphicsDrivers/GraphicsDriver.h"
#include "Utils/Cartesian.h"
#include "String.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class BusController;
class Timer;

namespace GraphicsDrivers
{

class RA8873 : public GraphicsDriver
{
public:
    RA8873( BusController& bus, Timers::Factory& timerFactory ) __attribute__((nomips16));
    ~RA8873() __attribute__((nomips16));

    virtual void clearScreen( const String& windowName ) __attribute__((nomips16));
    virtual void clearLines( const String& windowName, int from, int len ) __attribute__((nomips16));
    virtual void clearAll() __attribute__((nomips16));

    virtual void paintGlyph( const String& windowName, int row, int col, char* glyphAttr, bool transparency = false, char charset = 0 ) __attribute__((nomips16));
    virtual void paintBitmap( const String& windowName, int row, int col, int yOffset, int height, int width, char* rgbBuf ) __attribute__((nomips16)) {}

    virtual int glyphHeight() __attribute__((nomips16));
    virtual int glyphWidth() __attribute__((nomips16));

    virtual void brightnessUp() __attribute__((nomips16));
    virtual void brightnessDown() __attribute__((nomips16));

    virtual bool error() __attribute__((nomips16));

    virtual int selfTest() __attribute__((nomips16));

    virtual void dumpPerformanceInfo() __attribute__((nomips16));

    virtual void panic( unsigned int address, unsigned int code ) __attribute__((nomips16)) __attribute__((noreturn));

private:
    inline void writeRegister( int addr, int val ) __attribute__((nomips16));

    inline void writeData( int data ) __attribute__((nomips16));

    void waitForCoreIdle() __attribute__((nomips16));
    void waitForWFIFOEmpty() __attribute__((nomips16));
    void waitForWFIFONotFull() __attribute__((nomips16));
    
    void reset() __attribute__((nomips16));
    void setPLLs() __attribute__((nomips16));
    void initRAM() __attribute__((nomips16));
    void checkStatus() __attribute__((nomips16));

    void setBrightness() __attribute__((nomips16));

    void displayCopyright() __attribute__((nomips16));

    void clearRegion( const Rectangle& clearRect, unsigned char colour = 0 ) __attribute__((nomips16));

    void setColours( unsigned char attribute ) __attribute__((nomips16));
    void setTransparency( bool transparency ) __attribute__((nomips16));
    void setTextPosition( int x, int y ) __attribute__((nomips16));

    void loadCGRAM() __attribute__((nomips16));

    BusController& m_bus;
    
    Timer* m_timer;

    Rectangle m_screenRect;

    const Window* m_currentWindow;
    int m_absX;
    int m_absY;
    unsigned char m_attribute;
    bool m_transparency;

    int m_brightness;

    int m_winSwitches;
    int m_attrSwitches;
    int m_transSwitches;
    int m_posSwitches;

    unsigned char m_warn;
    int m_warnCount;

    bool m_doLog;
};

} // namespace GraphicsDrivers

} // namespace Agape

#endif // AGAPE_GRAPHICS_DRIVERS_RA8873_H
