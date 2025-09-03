#ifndef AGAPE_GRAPHICS_DRIVER_H
#define AGAPE_GRAPHICS_DRIVER_H

#ifdef QT_CORE_LIB
#include <QWidget>
#endif // QT_CORE_LIB

#include "Utils/Cartesian.h"
#include "Allocator.h"
#include "Collections.h"
#include "String.h"
#include "Testable.h"

namespace Agape
{

#ifdef QT_CORE_LIB
class GraphicsDriver : public QWidget, public Testable
{
    Q_OBJECT
#else
class GraphicsDriver : public Testable
{
#endif // QT_CORE_LIB

public:
    struct Window
    {
        String m_name;
        Rectangle m_rect;
        bool m_visible;
    };

    static const bool transparent;

    virtual ~GraphicsDriver();

    virtual void clearScreen( const String& windowName ) = 0;
    virtual void clearLines( const String& windowName, int from, int len ) = 0;
    virtual void clearAll() = 0;

    virtual void paintGlyph( const String& windowName, int row, int col, char* glyphAttr, bool transparency = false, char charset = 0 ) = 0;
    virtual void paintBitmap( const String& windowName, int row, int col, int yOffset, int height, int width, char* rgbBuf ) = 0;

    virtual int glyphHeight() = 0;
    virtual int glyphWidth() = 0;

    virtual const Window* createWindow( const Window& window );
    virtual void setWindowVisible( const String& windowName, bool visible );
    virtual bool findWindow( const String& windowName, const Window*& window ) const;

    virtual void moveWindow( const String& windowName, const Point& origin );

    /// Finds the highest priority window which overlaps any of the draw area.
    virtual Vector< const Window* > findWindowsTopmostVisible( const Rectangle& drawRect ) const;
    bool isWindowTopmostVisible( const Rectangle& drawRect, const String& windowName ) const;

    /// If returning true, indicates the upper graphics layers should redraw
    /// themselves. redrawComplete() should be called following this redraw.
    virtual bool requestRedraw() { return false; };
    virtual void redrawComplete() {};

    virtual void brightnessUp() {};
    virtual void brightnessDown() {};

    virtual bool error() { return false; };

    virtual void dumpPerformanceInfo() {};

    /// Permits graphics to be updated during busy waiting, for platforms that
    /// would otherwise need to go back to the main loop (e.g. Qt). Shouldn't
    /// ordinarily be needed if we're returning to the main loop regularly.
    virtual void flush() {};

    virtual int selfTest() { return 0; };

    virtual void panic( unsigned int address, unsigned int code ) {};

protected:
    Vector< Window* > m_windows;
};

} // namespace Agape

#endif // AGAPE_GRAPHICS_DRIVER_H
