#ifndef AGAPE_GRAPHICS_DRIVERS_QTWIND_H
#define AGAPE_GRAPHICS_DRIVERS_QTWIND_H

#include "Utils/Cartesian.h"
#include "GraphicsDriver.h"
#include "String.h"

#include <QEventLoop>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QObject>
#include <QSettings>
#include <QTimer>

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

class Timer;

namespace GraphicsDrivers
{

class QtWind : public GraphicsDriver
{
    Q_OBJECT

public:
    QtWind( Timers::Factory& timerFactory );
    ~QtWind();

    virtual void clearScreen( const String& windowName );
    virtual void clearLines( const String& windowName, int from, int len );
    virtual void clearAll();

    virtual void paintGlyph( const String& windowName, int row, int col, char* glyphAttr, bool transparency = false, char charset = 0 );
    virtual void paintBitmap( const String& windowName, int row, int col, int yOffset, int height, int width, char* rgbBuf );

    virtual int glyphHeight();
    virtual int glyphWidth();

    virtual bool requestRedraw();
    virtual void redrawComplete();

    virtual void flush();

private:
    unsigned int combineColours( unsigned int fg, unsigned int bg, double fgDiv, double bgDiv );

    void adjustWindow();
    void autoScale();

    qreal m_height;
    qreal m_width;
    Rectangle m_screenRect;
    qreal m_scaleFactor;
    bool m_noPixelArt;
    bool m_sharp;
    QPixmap m_pixmap;
    qreal m_devicePixelRatio;
    qreal m_xoffset;
    qreal m_yoffset;

    int m_repaintCounter;
    bool m_requestRedraw;

    int m_screenshotIdx;

    const Window* m_currentWindow;

    Timer* m_osdTimer;

signals:
    void keyPressed( QKeyEvent* );

public slots:
    void displayChanged();
    void exitEventLoop();

protected:
    virtual void keyPressEvent( QKeyEvent* event );
    virtual void paintEvent( QPaintEvent* event );

private:
    QSettings m_settings;
    QEventLoop m_eventLoop;
    QTimer m_eventLoopTimer;
};

} // namespace GraphicsDrivers

} // namespace Agape

#endif // AGAPE_GRAPHICS_DRIVERS_QTWIND_H
