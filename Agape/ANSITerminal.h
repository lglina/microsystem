#ifndef AGAPE_ANSI_TERMINAL_H
#define AGAPE_ANSI_TERMINAL_H

#include "String.h"
#include "Terminal.h"

namespace Agape
{

class Character;
class GraphicsDriver;
class Timer;

class ANSITerminal : public Terminal
{
public:
    ANSITerminal( int width,
                  int height,
                  const String& windowName,
                  GraphicsDriver& graphicsDriver,
                  Timer& timer,
                  Terminal* drawTerminal = nullptr,
                  bool haveBuffer = true );

    virtual void consumeChar( char c,
                              bool scrollLock = false,
                              int charMode = 0,
                              char* drawMap = nullptr,
                              int mapValue = 0,
                              char charset = -1 );
    virtual void consumeChar( char c,
                              int width,
                              int widthOffset,
                              bool scrollLock = false,
                              int charMode = 0,
                              char* drawMap = nullptr,
                              int mapValue = 0,
                              char charset = -1 );
    
    virtual int countPrinting( const String& string ) const;

    static String colours( int bgColour, int fgColour );
    static String colour( int fgColour );
    static String reset();

    static String charset( int charset );
    static String resetCharset();

    static int fromTermColour( int termColour );

private:
    void parseSGRSequence();
    void parseCursorSequence();
    void parseSGRAttribute( int attribute );

    bool m_consumingSequence;
    String m_currentSequence;

    int m_ansiForegroundColour;
    int m_ansiBackgroundColour;
    bool m_sgrBold;
    bool m_sgrBlink;
    int m_charset;
};

} // namespace Agape

#endif // AGAPE_ANSI_TERMINAL_H
