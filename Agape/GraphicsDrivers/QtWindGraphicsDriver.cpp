#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Utils/Cartesian.h"
#include "Utils/LiteStream.h"
#include "ARGBColours.h"
#include "Avatars.h"
#include "QtWindGraphicsDriver.h"
#include "String.h"
#include "vga.h"

#include <QApplication>
#include <QFile>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QSettings>
#include <QStyle>
#include <QTimer>

#include <sstream>

namespace
{
    const qreal defaultScaleFactor( 1.0 );
    const bool defaultNoPixelArt( true );
    const bool defaultSharp( true );
    const int osdTimeout( 2000 ); // ms.
    const int repaintInterval( 0 ); // Set non-zero to slow down drawing for debugging.

    const double fgDivB0( 2.00 );
    const double bgDivB0( 1.15 );
    const double fgDivB1( 1.40 );
    const double bgDivB1( 1.40 );
    const double fgDivB2( 1.15 );
    const double bgDivB2( 2.00 );

    enum
    {
        _glyphHeight = 16,
        _glyphWidth = 8
    };
} // Anonymous namespace.

namespace Agape
{

namespace GraphicsDrivers
{

QtWind::QtWind( Timers::Factory& timerFactory ) :
  m_height( 480.0 ),
  m_width( 800.0 ),
  m_screenRect( 0, 0, m_height, m_width ),
  m_scaleFactor( defaultScaleFactor ),
#ifdef DEFAULT_NO_PIXEL_ART
  m_noPixelArt( true ),
#else
  m_noPixelArt( defaultNoPixelArt ),
#endif
  m_sharp( defaultSharp ),
  m_pixmap( m_width, m_height ),
  m_devicePixelRatio( this->devicePixelRatioF() ),
  m_xoffset( 0 ),
  m_yoffset( 0 ),
  m_repaintCounter( 0 ),
  m_requestRedraw( false ),
  m_screenshotIdx( 0 ),
  m_currentWindow( nullptr ),
  m_settings( "Agape", "TerminalSimulator" )
{
    connect( &m_eventLoopTimer, &QTimer::timeout, this, &QtWind::exitEventLoop );

    QApplication* qApplication( qApp );
    const QScreen* primaryScreen( qApplication->primaryScreen() );
    connect( primaryScreen, &QScreen::availableGeometryChanged, this, &QtWind::displayChanged );

    if( m_settings.contains( "scaleFactor" ) )
    {
        m_scaleFactor = m_settings.value( "scaleFactor" ).toReal();
    }
    else
    {
        autoScale();
        m_settings.setValue( "scaleFactor", m_scaleFactor );
    }

    if( m_settings.contains( "noPixelArt" ) )
    {
        m_noPixelArt = m_settings.value( "noPixelArt" ).toBool();
    }

    if( m_settings.contains( "sharp" ) )
    {
        m_sharp = m_settings.value( "sharp" ).toBool();
    }

    m_osdTimer = timerFactory.makeTimer();

    setStyleSheet( "background-color:black;" );
    adjustWindow();
    show();

    QPainter painter( &m_pixmap );
    QColor color( QRgba64::fromArgb32( 0xFF000000 ) );
    painter.setPen( color );
    painter.fillRect( QRectF( 0.0,
                              0.0,
                              m_width,
                              m_height ),
                      color );
    this->update();
}

QtWind::~QtWind()
{
    delete( m_osdTimer );
}

void QtWind::clearScreen( const String& windowName )
{
    const Window* thisWindow;
    bool foundThis( findWindow( windowName, thisWindow ) );

    // FIXME: This will clear over any upper windows!
    if( foundThis && thisWindow->m_visible )
    {
        Rectangle clearRect( thisWindow->m_rect );
        Rectangle clipRect( clearRect.clipTo( m_screenRect ) );
#ifdef LOG_WINDOWS
        if( ( clipRect.height() != clearRect.height() ) ||
            ( clipRect.width() != clearRect.width() ) )
        {
            LOG_DEBUG( "Clear screen at " + clearRect.dump() + " clipped to " + clipRect.dump() );
        }
#endif
        if( ( clipRect.height() > 0 ) &&
            ( clipRect.width() > 0 ) )
        {
            QPainter painter( &m_pixmap );
            QColor color( QRgba64::fromArgb32( 0xFF000000 ) );
            painter.setPen( color );
            painter.fillRect( QRectF( qreal( clipRect.originX() ),
                                    qreal( clipRect.originY() ),
                                    qreal( clipRect.width() ),
                                    qreal( clipRect.height() ) ),
                            color );
            this->update();
        }
    }
}

void QtWind::clearLines( const String& windowName, int from, int len )
{
    const Window* thisWindow;
    bool foundThis( findWindow( windowName, thisWindow ) );

    // FIXME: This will clear over any upper windows!
    if( foundThis && thisWindow->m_visible )
    {
        Rectangle clearRect( thisWindow->m_rect.originX(),
                             thisWindow->m_rect.originY() + ( _glyphHeight * from ),
                             _glyphHeight * len,
                             thisWindow->m_rect.width() );
        Rectangle clipRect( clearRect.clipTo( m_screenRect ) );
#ifdef LOG_WINDOWS
        if( ( clipRect.height() != clearRect.height() ) ||
            ( clipRect.width() != clearRect.width() ) )
        {
            LOG_DEBUG( "Clear lines at " + clearRect.dump() + " clipped to " + clipRect.dump() );
        }
#endif
        if( ( clipRect.height() > 0 ) &&
            ( clipRect.width() > 0 ) )
        {
            QPainter painter( &m_pixmap );
            QColor color( QRgba64::fromArgb32( 0xFF000000 ) );
            painter.setPen( color );
            painter.fillRect( QRectF( qreal( clipRect.originX() ),
                                      qreal( clipRect.originY() ),
                                      qreal( clipRect.width() ),
                                      qreal( clipRect.height() ) ),
                              color );
            this->update();
        }
    }
}

void QtWind::clearAll()
{
    QPainter painter( &m_pixmap );
    QColor color( QRgba64::fromArgb32( 0xFF000000 ) );
    painter.setPen( color );
    painter.fillRect( QRectF( qreal( 0 ),
                              qreal( 0 ),
                              qreal( m_width ),
                              qreal( m_height ) ),
                      color );
    this->update();
}

void QtWind::paintGlyph( const String& windowName, int row, int col, char* glyphAttr, bool transparency, char charset )
{
    if( m_currentWindow == nullptr || m_currentWindow->m_name != windowName )
    {
        if( !findWindow( windowName, m_currentWindow ) )
        {
            m_currentWindow = nullptr;
        }
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
#ifdef LOG_WINDOWS
        if( ( clipRect.height() != glyphRect.height() ) ||
            ( clipRect.width() != glyphRect.width() ) )
        {
            LOG_DEBUG( "Draw glyph at " + glyphRect.dump() + " clipped to " + clipRect.dump() );
        }
#endif
        if( ( clipRect.height() == _glyphHeight ) &&
            ( clipRect.width() == _glyphWidth ) )
        {
            if( isWindowTopmostVisible( Rectangle( m_currentWindow->m_rect.originX() + x,
                                                   m_currentWindow->m_rect.originY() + y,
                                                   _glyphHeight,
                                                   _glyphWidth ),
                                        m_currentWindow->m_name ) )
            {
                QPainter painter( &m_pixmap );

                int glyphIdx( (unsigned char)( *glyphAttr ) );
                int attr( (unsigned char)( *( glyphAttr + 1 ) ) );
                int fgidx( attr & 0x0F );
                int bgidx( attr >> 4 );
                QColor qFGColour( QRgba64::fromArgb32( ARGBColours[ fgidx ] ) );
                QColor qBGColour( QRgba64::fromArgb32( ARGBColours[ bgidx ] ) );

                if( m_noPixelArt )
                {
                    if( glyphIdx == 0xb0 )
                    {
                        unsigned int combined( combineColours( qFGColour.rgba(),
                                                               qBGColour.rgba(),
                                                               fgDivB0,
                                                               bgDivB0 ) );
                        qFGColour = QRgba64::fromArgb32( combined );
                        glyphIdx = 0xdb;
                    }
                    else if( glyphIdx == 0xb1 )
                    {
                        unsigned int combined( combineColours( qFGColour.rgba(),
                                                               qBGColour.rgba(),
                                                               fgDivB1,
                                                               bgDivB1 ) );
                        qFGColour = QRgba64::fromArgb32( combined );
                        glyphIdx = 0xdb;
                    }
                    else if( glyphIdx == 0xb2 )
                    {
                        unsigned int combined( combineColours( qFGColour.rgba(),
                                                               qBGColour.rgba(),
                                                               fgDivB2,
                                                               bgDivB2 ) );
                        qFGColour = QRgba64::fromArgb32( combined );
                        glyphIdx = 0xdb;
                    }
                }

                for( int yoff = 0; yoff < _glyphHeight; ++yoff )
                {
                    for( int xoff = 0; xoff < _glyphWidth; ++xoff )
                    {
                        bool foregroundpx( false );
                        if( charset == 0 )
                        {
                            foregroundpx = ( (vgaGlyphs[ ( glyphIdx * _glyphHeight ) + yoff ] >> ( ( _glyphWidth - 1 ) - xoff ) ) & 1 ) == 1;
                        }
                        else if( charset == 1 )
                        {
                            foregroundpx = ( (avatarGlyphs[ ( ( glyphIdx - 128 ) * _glyphHeight ) + yoff ] >> ( ( _glyphWidth - 1 ) - xoff ) ) & 1 ) == 1;
                        }

                        if( foregroundpx )
                        {
                            painter.setPen( qFGColour );
                            painter.drawPoint( QPointF( qreal( m_currentWindow->m_rect.originX() + x + xoff ),
                                                        qreal( m_currentWindow->m_rect.originY() + y + yoff ) ) );
                        }
                        else if( !transparency || ( ( attr >> 4 ) != 0 ) )
                        {
                            painter.setPen( qBGColour );
                            painter.drawPoint( QPointF( qreal( m_currentWindow->m_rect.originX() + x + xoff ),
                                                        qreal( m_currentWindow->m_rect.originY() + y + yoff ) ) );
                        }
                    }
                }

                this->update();

                if( repaintInterval > 0 )
                {
                    ++m_repaintCounter;
                    if( m_repaintCounter == repaintInterval )
                    {
                        this->repaint();
                        m_repaintCounter = 0;
                    }
                }
            }
        }
    }
}

void QtWind::paintBitmap( const String& windowName, int row, int col, int yOffset, int height, int width, char* rgbBuf )
{
    const Window* thisWindow;
    bool foundThis( findWindow( windowName, thisWindow ) );

    if( foundThis )
    {
        int x( col * _glyphWidth );
        int y( ( row * _glyphHeight ) + yOffset );

        //Window* topmostVisibleWindow;
        //bool foundTopmost( findWindowTopmostVisible( thisWindow->m_x + x, thisWindow->m_y + y, 1, width, &topmostVisibleWindow ) );

        //if( foundTopmost && ( topmostVisibleWindow == thisWindow ) ) // implies thisWindow->m_visible == true
        //{
            QPainter painter( &m_pixmap );

            char* bufferPtr( rgbBuf );
            for( int xoff = 0; xoff < width; ++xoff )
            {
                QColor color( QRgba64::fromRgba( *(unsigned char*)( bufferPtr ),
                                                 *(unsigned char*)( bufferPtr + 1 ),
                                                 *(unsigned char*)( bufferPtr + 2 ),
                                                 0xFF ) );
                bufferPtr += 3;

                painter.setPen( color );
                painter.drawPoint( QPointF( qreal( thisWindow->m_rect.originX() + x + xoff ),
                                            qreal( thisWindow->m_rect.originY() + y ) ) );
            }

            this->repaint();
        //}
    }
}

int QtWind::glyphHeight()
{
    return _glyphHeight;
}

int QtWind::glyphWidth()
{
    return _glyphWidth;
}

bool QtWind::requestRedraw()
{
    return m_requestRedraw;
}

void QtWind::redrawComplete()
{
    m_requestRedraw = false;
}

void QtWind::flush()
{
    // See comments in QtWebSocketsConnection for details...
    m_eventLoopTimer.setSingleShot( true );
    m_eventLoopTimer.start( 0 );
    m_eventLoop.exec();
}

unsigned int QtWind::combineColours( unsigned int fg, unsigned int bg, double fgDiv, double bgDiv )
{
    // Scale and combine foreground and background.
    unsigned int blue = ( fg & 0x000000FF ) / fgDiv;
    unsigned int green = ( ( fg & 0x0000FF00 ) >> 8 ) / fgDiv;
    unsigned int red = ( ( fg & 0x00FF0000 ) >> 16 ) / fgDiv;
    blue += ( bg & 0x000000FF ) / bgDiv;
    green += ( ( bg & 0x0000FF00 ) >> 8 ) / bgDiv;
    red += ( ( bg & 0x00FF0000 ) >> 16 ) / bgDiv;
    
    // Clamp to saturation point.
    if( blue > 0xFF ) blue = 0xFF;
    if( green > 0xFF ) green = 0xFF;
    if( red > 0xFF ) red = 0xFF;

    // Combine to ARGB.
    unsigned int combined( 0xFF000000 ); // Opaque
    combined += blue;
    combined += green << 8;
    combined += red << 16;

    return combined;
}

void QtWind::adjustWindow()
{
#ifndef __EMSCRIPTEN__
    // Set the widget to be the correct size in device pixels.
    setFixedSize( m_width / m_devicePixelRatio * m_scaleFactor, m_height / m_devicePixelRatio * m_scaleFactor );
#else
    // Qt wasm seems to expand us to cover the entire canvas and doesn't
    // respect fixedSize or move(), so let's expand ourselves, then paint in
    // the middle of the canvas.
    QApplication* qApplication( qApp );
    const QScreen* primaryScreen( qApplication->primaryScreen() );
    QRect availGeom( primaryScreen->availableGeometry() );
    setFixedSize( availGeom.width(), availGeom.height() );
    m_xoffset = ( availGeom.width() / 2 ) - ( ( ( m_width * m_scaleFactor ) / m_devicePixelRatio ) / 2 );
    m_yoffset = ( availGeom.height() / 2 ) - ( ( ( m_height * m_scaleFactor ) / m_devicePixelRatio ) / 2 );

    LiteStream stream;
    stream << "Avail x: " << availGeom.x() << " y: " << availGeom.y()
           << " height: " << availGeom.height() << " width: " << availGeom.width()
           << " This widget x: " << rect().x() << " y: " << rect().y()
           << " height: " << rect().height() << " width: " << rect().width()
           << " vlcd height: " << m_height << " width: " << m_width
           << " DPR: " << m_devicePixelRatio << " scaleFactor: " << m_scaleFactor
           << " xoff: " << m_xoffset << " yoff: " << m_yoffset;
    LOG_DEBUG( stream.str() );
#endif
}

void QtWind::autoScale()
{
    QApplication* qApplication( qApp );
    const QScreen* primaryScreen( qApplication->primaryScreen() );
    QRect availGeom( primaryScreen->availableGeometry() );
    if( ( ( availGeom.height() * m_devicePixelRatio ) >= ( m_height * 3.0 ) ) &&
        ( ( availGeom.width() * m_devicePixelRatio ) >= ( m_width * 3.0 ) ) )
    {
        m_scaleFactor = 3.0;
    }
    else if( ( ( availGeom.height() * m_devicePixelRatio ) >= ( m_height * 2.5 ) ) &&
             ( ( availGeom.width() * m_devicePixelRatio ) >= ( m_width * 2.5 ) ) )
    {
        m_scaleFactor = 2.5;
    }
    else if( ( ( availGeom.height() * m_devicePixelRatio ) >= ( m_height * 2.0 ) ) &&
             ( ( availGeom.width() * m_devicePixelRatio ) >= ( m_width * 2.0 ) ) )
    {
        m_scaleFactor = 2.0;
    }
    else if( ( ( availGeom.height() * m_devicePixelRatio ) >= ( m_height * 1.5 ) ) &&
             ( ( availGeom.width() * m_devicePixelRatio ) >= ( m_width * 1.5 ) ) )
    {
        m_scaleFactor = 1.5;
    }
    else
    {
        m_scaleFactor = 1.0;
    }
}

void QtWind::displayChanged()
{
    adjustWindow();
    this->repaint();
}

void QtWind::exitEventLoop()
{
    m_eventLoop.exit();
}

void QtWind::keyPressEvent( QKeyEvent* event )
{
    if( event->key() == Qt::Key_Backslash )
    {
#ifndef __EMSCRIPTEN__
        std::ostringstream filename;
        filename << "snap-" << m_screenshotIdx++ << ".png";
        QFile file( filename.str().c_str() );
        file.open( QIODevice::WriteOnly );
        m_pixmap.save( &file, "PNG" );
        file.close();
#endif
    }
    else if( ( event->modifiers() & Qt::ControlModifier ) &&
             ( event->key() == Qt::Key_Minus ) )
    {
        if( m_scaleFactor > 0.5 )
        {
            m_scaleFactor -= 0.25;
            m_osdTimer->reset();
            adjustWindow();
            this->repaint();
            m_settings.setValue( "scaleFactor", m_scaleFactor );
        }
    }
    else if( ( event->modifiers() & Qt::ControlModifier ) &&
             ( event->key() == Qt::Key_Equal ) )
    {
        m_scaleFactor += 0.25;
        m_osdTimer->reset();
        adjustWindow();
        this->repaint();
        m_settings.setValue( "scaleFactor", m_scaleFactor );
    }
    else if( ( event->modifiers() & Qt::ControlModifier ) &&
             ( event->key() == Qt::Key_0 ) )
    {
        m_noPixelArt = !m_noPixelArt;
        m_osdTimer->reset();
        this->repaint();
        m_requestRedraw = true;
        m_settings.setValue( "noPixelArt", m_noPixelArt );
    }
    else if( ( event->modifiers() & Qt::ControlModifier ) &&
             ( event->key() == Qt::Key_9 ) )
    {
        m_sharp = !m_sharp;
        m_osdTimer->reset();
        this->repaint();
        m_requestRedraw = true;
        m_settings.setValue( "sharp", m_sharp );
    }
    else
    {
        // Send through to QtWindInputDevice.
        emit keyPressed( event );
    }
}

void QtWind::paintEvent( QPaintEvent* event )
{
    QPainter painter( this );
    // Scale pixmap with the inverse of the high DPI transform.
    if( m_sharp )
    {
        painter.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform, false );
    }
    else
    {
        painter.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
    }
    painter.scale( qreal( 1.0 ) / m_devicePixelRatio * m_scaleFactor, qreal( 1.0 ) / m_devicePixelRatio * m_scaleFactor );
    painter.drawPixmap( QPointF( ( m_xoffset / m_scaleFactor ) * m_devicePixelRatio, ( m_yoffset / m_scaleFactor ) * m_devicePixelRatio ), m_pixmap );

    if( m_osdTimer->ms() < osdTimeout )
    {
        painter.setPen( QColor( 255, 255, 255, 255 ) );
        
        {
        LiteStream stream;
        stream << "Scale " << m_scaleFactor;
        painter.drawText( QPointF( 10, 20 ), stream.str().c_str() );
        }
        
        {
        LiteStream stream;
        if( m_sharp )
        {
            stream << "(sharp)";
        }
        else
        {
            stream << "(smoothed)";
        }
        if( m_noPixelArt )
        {
            stream << " (pixel art disabled)"; 
        }
        else
        {
            stream << " (pixel art enabled)";
        }
        painter.drawText( QPointF( 10, 40 ), stream.str().c_str() );
        }
    }
}

} // namespace GraphicsDrivers

} // namespace Agape
