#ifndef AGAPE_TERMINAL_H
#define AGAPE_TERMINAL_H

#include "Allocator.h"
#include "Collections.h"
#include "Runnable.h"
#include "String.h"

namespace Agape
{

class Asset;
class GraphicsDriver;
class Timer;

class Terminal : public Runnable
{
public:
    class Character
    {
    public:
        Character() :
            m_row( -1 ),
            m_col( -1 ),
            m_character( -1 ),
            m_attributes( 0 ),
            m_charset( 0 )
        {
        }

        char m_row;
        char m_col;
        char m_character;
        char m_attributes;
        char m_charset;
    };

    enum CharModes
    {
        noCharmode = 0,
        blit = 1,                       // Draw foreground pixels only. Don't draw background pixels.
        transient = 2,                  // Draw, but don't save for redrawing.
        preserveBackground = 4,         // Draw background and foreground pixels, but with a background colour the same as for the existing character.
        whitespaceTransparency = 8,     // Don't draw space characters with a black background.
        force = 16,                     // Force drawing character even if we think it's already drawn.
        literal = 32,                   // Draw actual character, rather than treating it as a control character.
        ANSI = 64                       // Interpret CR, LF and ANSI escape as control characters, don't interpret BS.
    };

    static const int colBlack;
    static const int colBlue;
    static const int colGreen;
    static const int colCyan;
    static const int colRed;
    static const int colMagenta;
    static const int colBrown;
    static const int colGrey;
    static const int colDarkGrey;
    static const int colBrightBlue;
    static const int colBrightGreen;
    static const int colBrightCyan;
    static const int colBrightRed;
    static const int colBrightMagenta;
    static const int colYellow;
    static const int colWhite;

    static const int noMaxHeight;
    static const int noMaxWidth;
    static const bool hCentre;
    static const bool noHCentre;
    static const bool vCentre;
    static const bool noVCentre;
    static const bool scrollLock;
    static const bool scrollUnlock;
    static const int noMaxRow;
    static const int atCurrentRow;
    static const int atCurrentCol;
    static const bool scrollUp;
    static const bool scrollDown;

    static const char avatarCharset;

    Terminal( int width,
              int height,
              const String& windowName,
              GraphicsDriver& graphicsDriver,
              Timer& timer,
              Terminal* drawTerminal = nullptr,
              bool haveBuffer = true );
    virtual ~Terminal();

    int width() const;
    int height() const;

    int row() const;
    int col() const;

    void repaint();
    void repaint( int row, int col, int height, int width );
    
    void clearScreen( bool resetAttributes = false );
    void clearLines( int from, int len, bool resetAttributes = false );

    void fillScreen( char c, char attributes, int from = 0, int len = -1, int charMode = 0 );
    void scrollScreen( bool down, int from = 0, int len = -1 );

    void transientBlank();

    void setAttributes( char attributes );

    virtual void consumeNext( int row, int col, char attributes = 0x07 );

    // N.B. Cursor semantics:
    // In a repaint, all cursors in the repainted area (if any) are redrawn.
    // This is different than when calling consumeX(), where cursors aren't
    // redraw until redrawCursors() is called. This is so that we're not
    // constantly blitting cursors while we're trying to build up a new screen.
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

    virtual void consumeString( const String& string,
                                bool scrollLock = false,
                                int charMode = 0,
                                char* drawMap = nullptr,
                                int mapValue = 0 );

    void insertAtCursor( char c,
                         const String& cursorName,
                         int fieldStart = 0,
                         int fieldWidth = -1 );
    void backspaceAtCursor( const String& cursorName,
                            int fieldStart = 0,
                            int fieldWidth = -1 );
    void insertAtCursor( char c );
    void backspaceAtCursor();

    void consumeAsset( const Asset& asset,
                       int len,
                       bool scrollLock = false,
                       int charMode = 0,
                       char* drawMap = nullptr,
                       int mapValue = 0 );
    int consumeAsset( const Asset& asset,
                      int assetOffset,
                      int len,
                      int width,
                      int widthOffset,
                      int maxRow,
                      bool scrollLock = false,
                      int charMode = 0,
                      char* drawMap = nullptr,
                      int mapValue = 0 );
    
    void createSprite( const String& name,
                       const String& assetName,
                       const Asset& asset,
                       int len,
                       int row,
                       int col,
                       int height,
                       int width,
                       int charMode = 0,
                       int numFrames = 1 );
    bool isSprite( const String& name ) const;
    bool spriteData( const String& name,
                     String& assetName,
                     int& row,
                     int& col,
                     int& height,
                     int& width );
    bool spriteMap( const String& name,
                    char*& spriteMap );
    void moveSprite( const String& name,
                     int row,
                     int col );
    void deleteSprite( const String& name );
    void deleteAllSprites( bool doRepaint = true );
    void collideSprite( int row,
                        int col,
                        int height,
                        int width,
                        Vector< String >& names ) const;

    void consumeGraphicalAsset( const Asset& asset, int len );

    void getBaseCharAt( int row, int col, char& character, char& attributes ) const;

    virtual int countPrinting( const String& string ) const;
    int printFormatted( const String& string );
    int printFormatted( const String& string,
                        int row,
                        int col,
                        int maxHeight,
                        int maxWidth,
                        bool hcentre,
                        bool vcentre,
                        char attributes = 0x07,
                        int charmode = 0 );

    void createCursor( const String& name, int row, int col, char glyph, char attributes, char charset = 0 );
    void deleteCursor( const String& name );
    void deleteAllCursors();
    void moveCursor( const String& name, int row, int col, char newGlyph = 0, char newAttributes = -1 );
    bool getCursor( const String& name, int& row, int& col ) const;
    void setCursorVisible( const String& name, bool visible );
    void setCursorsVisible( bool visible );
    void redrawCursors();

    void setCursorVariant( int variant );

    void enableTerminalCursor( bool enabled );

    void clipCopy( int row, int col, int height, int width );
    void clipCut( int row, int col, int height, int width );
    void clipPaste( int row, int col, int cropRow = -1, int cropCol = -1 );

    static char attributes( const String& colour );
    static char attributes( int bgColour, int fgColour );

    const char* buffer() const;

    void flush();

    void run(); // Update animations.

protected:
    void putCharacter( Character c, int charMode = 0 );
    void eraseCharacter( int charMode );

    bool isTransparentWhitespace( Character c, int charMode );
    bool isTransparentWhitespace( char character, char attributes, int charMode );

    int m_width;
    int m_height;
    int m_row;
    int m_col;
    int m_characterAttributes;
    String m_windowName;
    bool m_terminalCursorEnabled;

private:
    class Cursor
    {
    public:
        String m_name;
        int m_row;
        int m_col;
        char m_glyph;
        char m_charset;
        char m_attributes;
        bool m_visible;
    };

    class SpriteBuffer
    {
    public:
        SpriteBuffer( int height, int width, int numFrames );
        ~SpriteBuffer();

        char* m_buffer;
        char* m_map;
        int m_users;
    
    private:
        SpriteBuffer( const SpriteBuffer& other ) {};
        SpriteBuffer& operator=( const SpriteBuffer& other ) { return *this; };
    };

    // Use char instead of int to save on memory if we have a lot of
    // blitted/animated characters.
    class Sprite
    {
    public:
        Sprite( const String& name,
                const String& assetName,
                char row,
                char col,
                char height,
                char width,
                int charMode,
                char numFrames );
        
        ~Sprite();

        String m_name;
        String m_assetName;
        char m_row;
        char m_col;
        char m_height;
        char m_width;
        int m_charMode;
        char m_numFrames;
        char m_currentFrame;

        bool m_newBuffer;
        char* m_buffer;
        char* m_map;

        static Map< String, SpriteBuffer* > m_spriteBuffers;
    
    private:
        Sprite( const Sprite& other ) {};
        Sprite& operator=( const Sprite& other ) { return *this; };
    };

    void drawCursor( const Cursor& cursor );

    void drawSprite( const Sprite& sprite,
                     int startRow = -1,
                     int startCol = -1,
                     int drawHeight = -1,
                     int drawWidth = -1 );

    GraphicsDriver& m_graphicsDriver;
    Timer& m_timer;
    Terminal* m_drawTerminal;

    bool m_haveBuffer;
    char* m_buffer;

    Vector< Cursor > m_cursors;
    Vector< Sprite* > m_sprites;

    int m_clipHeight;
    int m_clipWidth;

    int m_cursorVariant;
};

} // namespace Agape

#endif // AGAPE_TERMINAL_H
