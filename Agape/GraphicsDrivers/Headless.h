#ifndef AGAPE_GRAPHICS_DRIVERS_HEADLESS_H
#define AGAPE_GRAPHICS_DRIVERS_HEADLESS_H

#include "GraphicsDriver.h"
#include "String.h"

namespace Agape
{

namespace GraphicsDrivers
{

class Headless : public GraphicsDriver
{
public:
    virtual void clearScreen( const String& windowName );
    virtual void clearLines( const String& windowName, int from, int len );
    virtual void clearAll();

    virtual void paintGlyph( const String& windowName, int row, int col, char* glyphAttr, bool transparency = false, char charset = 0 );
    virtual void paintBitmap( const String& windowName, int row, int col, int yOffset, int height, int width, char* rgbBuf );

    virtual int glyphHeight();
    virtual int glyphWidth();
};

} // namespace GraphicsDrivers

} // namespace Agape

#endif // AGAPE_GRAPHICS_DRIVERS_HEADLESS_H
