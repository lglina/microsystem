#ifndef AGAPE_EDITOR_EDITOR_H
#define AGAPE_EDITOR_EDITOR_H

#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "EditorChunk.h"
#include "Runnable.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{
class Factory;
} // namespace AssetLoaders

namespace Timers
{
class Factory;
} // namespace Timers

class AssetLoader;
class Terminal;
class Timer;

namespace Editor
{

namespace Highlighters
{
class Factory;
} // namespace Highlighters

class Editor : public Runnable
{
public:
    enum OpenMode
    {
        modeNone = 0,
        modeRead = 1,
        modeWrite = 2,
        modeCreate = 4
    };

    Editor( Terminal& terminal,
            AssetLoaders::Factory& assetLoaderFactory,
            Highlighters::Factory* highlighterFactory,
            const World::Coordinates& coordinates,
            const String& assetName,
            const String& instanceName,
            const String& displayName,
            const String& linkedItem,
            const String& keyGuide,
            Timers::Factory& timerFactory,
            Terminal* errorsTerminal = nullptr,
            Terminal* debugLinesTerminal = nullptr,
            Terminal* debugChunksTerminal = nullptr );
    ~Editor();

    bool open( int openMode );
    bool modified() const;
    bool close( bool save );
    bool close( bool save, const String& as );

    void getPosition( int& offset, int& row, int& col );
    bool setPosition( int offset, int row, int col );

    void draw( bool highlight = false, bool repaginate = true );

    void consumeCharacter( char c );

    bool error();

    virtual void run();

private:
    struct Line
    {
        int m_offset;
        char m_index;
        char m_logicalLine;
        short m_length;
    };

    enum ReadMode
    {
        readModeLen,
        readModeNewline
    };

    void paginate();
    void _draw( bool highlight, bool fillScreen = false );

    int read( String& line, int from, bool& newline, bool& eof, bool& error );
    int read( String& line, int from, int len, bool& error );
    
    int _read( String& line, int from, int len, bool& newline, bool& eof, enum ReadMode readMode, bool& error );

    void debugDrawLines();
    void debugDrawChunks();

    bool tryMoveCursor( int row, int col );

    bool scrollUp();
    bool scrollDown();

    void insertCharacter( char c );
    void deleteCharacter( bool previous );

    bool fastInsert( char c );
    bool fastBackspace();

    List< Line >::iterator getLineByRow( int row );
    int getOffsetAtCursor();
    void getCursorPositionForOffset( int offset, int& row, int& col );

    Terminal& m_terminal;
    AssetLoaders::Factory& m_assetLoaderFactory;
    Highlighters::Factory* m_highlighterFactory;
    World::Coordinates m_coordinates;
    String m_assetName;
    String m_instanceName;
    String m_displayName;
    String m_linkedItem;
    String m_keyGuide;
    Terminal* m_errorsTerminal;
    Terminal* m_debugLinesTerminal;
    Terminal* m_debugChunksTerminal;

    Timer* m_idleTimer;
    bool m_needsHighlight;
    bool m_needsRedraw;

    int m_openMode;

    bool m_modified;

    bool m_error;

    AssetLoader* m_assetLoader;

    bool m_unixEOL;

    List< Chunk > m_chunks;

    List< Line > m_lines;
    List< Line >::iterator m_pageTopLine;

    int m_cursorRow;
    int m_cursorCol;
};

} // namespace Editor

} // namespace Agape

#endif // AGAPE_EDITOR_EDITOR_H
