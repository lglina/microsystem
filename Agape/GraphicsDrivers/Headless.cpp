#include "Headless.h"
#include "GraphicsDriver.h"
#include "String.h"

namespace Agape
{

namespace GraphicsDrivers
{

void Headless::clearScreen( const String& windowName )
{
}

void Headless::clearLines( const String& windowName, int from, int len )
{
}

void Headless::clearAll()
{
}

void Headless::paintGlyph( const String& windowName, int row, int col, char* glyphAttr, bool transparency, char charset )
{
}

void Headless::paintBitmap( const String& windowName, int row, int col, int yOffset, int height, int width, char* rgbBuf )
{
}

int Headless::glyphHeight()
{
    return 16;
}

int Headless::glyphWidth()
{
    return 8;
}

} // namespace GraphicsDrivers

} // namespace Agape
