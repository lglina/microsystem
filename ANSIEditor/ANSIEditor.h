#ifndef AGAPE_ANSI_EDITOR_H
#define AGAPE_ANSI_EDITOR_H

#include "Assets/SAUCE.h"
#include "World/WorldCoordinates.h"
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
class Clock;
class Terminal;
class Timer;

namespace ANSIEditor
{

class ANSIEditor : public Runnable
{
public:
    enum OpenMode
    {
        modeNone = 0,
        modeRead = 1,
        modeWrite = 2,
        modeCreate = 4
    };

    ANSIEditor( Terminal& terminal,
                Terminal& toolboxTerminal,
                Terminal& statusTerminal,
                AssetLoaders::Factory& assetLoaderFactory,
                const World::Coordinates& coordinates,
                const String& assetName,
                const String& author,
                Timers::Factory& timerFactory,
                Clock& clock );
    ~ANSIEditor();

    bool open( int openMode );
    void draw();
    void redraw();

    void consumeCharacter( char c );

    const String& assetName() const;

    const String& getTemplateName() const;
    void setTemplateName( const String& templateName );

    int fgColour() const;
    int bgColour() const;
    void setFGColour( int fgColour );
    void setBGColour( int bgColour );

    bool modified();
    bool close( bool doSave );
    bool close( bool doSave, const String& as );
    bool save();
    bool save( const String& as );

    virtual void run();

private:
    void defineFrame();
    void drawFrame( bool erase = false );
    
    void drawToolbox();
    void updateToolbox( int oldColour, int newColour, bool fg );
    void drawToolboxMarker( int colour, bool fg, bool on );

    void drawCharacters();

    void drawFlags();

    bool tryMoveCursor( int row, int col );
    void tryBackspace();
    void tryType( char c );

    bool isForbiddenChar( char c );

    void switchHeightMode();
    char charFromHeight( int height );
    void drawAllHeights();
    void setHeight( char c );

    void tryDragSelection( int row, int col );
    void updateSelection();
    void cursorCreateOrMove( const String& name, int row, int col, char glyph, char attributes );
    void cancelSelection();

    void tryResize( int height, int width );

    void trySetAnimationFrames( int animFrames );

    void clipCopy();
    void clipCut();
    void clipPaste();

    void clipCopyHeightMap( bool doZero );
    void clipPasteHeightMap();

    void undo();

    void setSAUCEDefaults();

    Terminal& m_terminal;
    Terminal& m_toolboxTerminal;
    Terminal& m_statusTerminal;
    AssetLoaders::Factory& m_assetLoaderFactory;
    const World::Coordinates& m_coordinates;
    String m_assetName;
    String m_author;
    Clock& m_clock;

    Timer* m_timer;

    int m_openMode;

    AssetLoader* m_currentAssetLoader;

    int m_row;
    int m_col;
    int m_cursorAttributes;

    int m_frameX;
    int m_frameY;

    int m_height;
    int m_width;

    bool m_blit;
    bool m_sprite;
    bool m_animate;
    int m_animFrames;
    String m_templateName;

    Assets::SAUCE m_sauce;

    int m_fgColour;
    int m_bgColour;

    int m_set1Idx;
    int m_set2Idx;
    bool m_literal;

    bool m_haveSelection;
    int m_selectionX;
    int m_selectionY;
    int m_selectionHeight;
    int m_selectionWidth;
    int m_clipHeight;
    int m_clipWidth;

    char* m_heightMap;
    char* m_clipHeightMap;
    bool m_showHeights;

    bool m_modified;

    bool m_haveUndo;
    bool m_undoBackspace;
    int m_undoRow;
    int m_undoCol;
    char m_undoCharacter;
    char m_undoAttribute;
};

} // namespace ANSIEditor

} // namespace Agape

#endif // AGAPE_ANSI_EDITOR_H
