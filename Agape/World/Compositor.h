#ifndef AGAPE_COMPOSITOR_H
#define AGAPE_COMPOSITOR_H

#include "Allocator.h"
#include "AssetLoaders/Factories/BakedAssetLoaderFactory.h"
#include "Collections.h"
#include "Direction.h"
#include "PresenceLoaders/PresenceLoader.h"
#include "Runnable.h"
#include "Scene.h"
#include "SceneLoaders/SceneLoader.h"
#include "SceneLoaders/SceneRequest.h"
#include "String.h"
#include "World/ScenePresence.h"
#include "World/User.h"
#include "World/WorldCoordinates.h"

namespace Agape
{

namespace Audio
{
    class MIDIPlayer;
}

namespace Carlo
{
    class ProgramManager;
} // namespace Carlo

namespace SceneLoaders
{
    class Factory;
} // namespace SceneLoaders

namespace AssetLoaders
{
    class Factory;
} // namespace AssetLoaders

namespace PresenceLoaders
{
    class Factory;
} // namespace PresenceLoaders

namespace Linda2
{
class TupleRouter;
} // namespace Linda2

namespace Timers
{
class Factory;
} // namespace Timers

class Clock;
class PresenceRequest;
class Terminal;
class Timer;
class Worldbook;

namespace World
{

class Compositor : public Runnable
{
public:
    Compositor( Terminal& terminal,
                SceneLoaders::Factory& sceneLoaderFactory,
                AssetLoaders::Factory& assetLoaderFactory,
                PresenceLoaders::Factory& presenceLoaderFactory,
                Worldbook& worldbook,
                Linda2::TupleRouter& tupleRouter,
                Carlo::ProgramManager& programManager,
                AssetLoaders::Factory& programAssetLoaderFactory,
                Timers::Factory& timerFactory,
                Clock& clock,
                Audio::MIDIPlayer& midiPlayer );
    virtual ~Compositor();

    int height() const;
    int width() const;

    void render( const Coordinates& newCoordinates ); // Calls depart() first.
    void render(); // Re-draws existing scene.
    void depart();

    void setUser( const User& user );
    void setPosition( int row, int col, enum Direction::_Direction direction = Direction::none );
    void setPositionToPrevious();
    int positionRow() const;
    int positionCol() const;
    void walk( enum Direction::_Direction direction, bool& atEdge );

    void createSprite( const String& name, const String& assetName, int row, int col );
    bool isSprite( const String& name ) const;
    void moveSprite( const String& name, int row, int col );
    bool spriteData( const String& name, String& assetName, int& row, int& col, int& height, int& width );
    void deleteSprite( const String& name );
    void spriteToScene( const String& name, const String& action, bool immediate = false );

    void createItem( const String& name, const String& action, int row, int col, bool immediate = false );
    void moveItem( const String& snowflake, int row, int col, enum Direction::_Direction direction = Direction::none, bool keyboard = false, bool immediate = false );
    void moveItem( const String& snowflake, enum Direction::_Direction direction );
    void updateItem( const String& snowflake, const String& name, const String& action, bool immediate = false, bool haveAction = true );
    void setItemFlags( const String& snowflake, enum SceneItem::Flags flags, bool immediate = false );
    void deleteItem( const String& snowflake, bool immediate = false );
    void transportItem( const String& snowflake, int x, int y, int row, int col );
    void raiseItem( const String& snowflake, bool immediate = false );
    void lowerItem( const String& snowflake, bool immediate = false );

    void moveCurrentItem( int row, int col, enum Direction::_Direction direction = Direction::none, bool keyboard = false, bool immediate = false );
    void updateCurrentItem( const String& name, const String& action, bool immediate = false, bool haveAction = true );
    void deleteCurrentItem( bool immediate = false );
    void raiseCurrentItem( bool immediate = false );
    void lowerCurrentItem( bool immediate = false );

    void addTextCurrentItem();
    void deleteTextCurrentItem();

    void drawTextItem( const SceneItem& sceneItem, const String& text, bool clearFirst = false );

    const ScenePresence* findPresenceBySnowflake( const String& snowflake ) const;

    void createPerson( const String& name, int glyph, int attributes, int row, int col );
    void movePerson( const String& snowflake, int row, int col, enum Direction::_Direction direction = Direction::none );
    void movePerson( const String& snowflake, enum Direction::_Direction direction );
    void deletePerson( const String& snowflake );

    void setCursorVariant( int variant );

    const SceneItem* selectLast();
    const SceneItem* selectNext();
    const SceneItem* selectPrevious();
    const SceneItem* selectDirection( enum Direction::_Direction direction );
    const SceneItem* selectClosest( int row, int col );
    const SceneItem* selectBy( const String& snowflake );
    void selectNone();

    Set< const SceneItem* > findNearbyItems( const SceneItem* sceneItem ) const;
    Set< const ScenePresence* > findNearbyUsers( const SceneItem* sceneItem ) const;

    // FIXME: Add more, consider all returning Sets?
    Set< const SceneItem* > findItems( const String& name ) const;
    const SceneItem* findItemBySnowflake( const String& snowflake ) const;
    const SceneItem* findItemByAction( const String& action ) const;
    const SceneItem* itemAt( int row, int col, int height, int width ) const;

    int getHeightAt( int row, int col );
    void setHeightAt( int row, int col, int newHeight );

    void actionMessage();

    void requestTeleport( Coordinates& teleportCoordinates, int row, int col );
    bool teleportRequested();
    void getTeleportDest( Coordinates& teleportCoordinates, int& row, int& col );
    void clearTeleportRequested();

    bool hasSceneItemAttribute( const String& snowflake,
                                const String& name );
    bool createSceneItemAttribute( const String& snowflake,
                                   const String& name );
    bool loadSceneItemAttribute( const String& snowflake,
                                 const String& name,
                                 Value& value );
    bool saveSceneItemAttribute( const String& snowflake,
                                 const String& name,
                                 const Value& value );
    bool deleteSceneItemAttributes( const String& snowflake );

    void invalidateAllCached();
    void invalidateCached( const Vector< struct SceneLoader::InvalidatedAsset >& invalidatedAssets,
                           bool& didRender );
    void notifyInvalidated( const String& name,
                            const String& type,
                            bool& didRender );

    void lockScene( bool lock );

    void setCursorsVisible( bool visible );
    void clearScreen();

    virtual void run();

private:
    enum ActionType
    {
        loadAction,
        renderAction,
        bumpAction
    };

    void tileBackground();

    void _drawTextItem( const SceneItem& sceneItem, const String& text, bool clearFirst = false );

    void getAssetDimensions( const String& assetName, int& height, int& width );
    
    void loadTemplateProgram( const String& itemSnowflake, const String& templateName );
    void loadLinkedProgram( const String& itemSnowflake );

    void tryWalk( enum Direction::_Direction direction, int newRow, int newCol, int glyphOffset1, int glyphOffset2 );

    void updateScene( Vector< SceneRequest >& requests );
    void updatePresences( Vector< PresenceRequest >& requests );

    void collideBaseAndSprites( int row,
                                int col,
                                int height,
                                int width,
                                Set< const SceneItem* >& sceneItems,
                                const SceneItem* colliderSceneItem = nullptr ) const;
    void collideBaseAndSprites( int row,
                                int col,
                                Set< const SceneItem* >& sceneItems,
                                const SceneItem* colliderSceneItem = nullptr ) const;

    void collideItem( const SceneRequest& request );
    void collidePerson( const PresenceRequest& request );

    void performActionOnAll( enum ActionType actionType );
    void tryPerformAction( const SceneItem& sceneItem, enum ActionType actionType );
    void performAction( const String& actionString, const SceneItem& sceneItem );

    Terminal& m_terminal;
    SceneLoaders::Factory& m_sceneLoaderFactory;
    AssetLoaders::Factory& m_assetLoaderFactory;
    PresenceLoaders::Factory& m_presenceLoaderFactory;
    Worldbook& m_worldbook;
    Linda2::TupleRouter& m_tupleRouter;
    Carlo::ProgramManager& m_programManager;
    AssetLoaders::Factory& m_programAssetLoaderFactory;

    Clock& m_clock;

    Audio::MIDIPlayer& m_midiPlayer;

    AssetLoaders::Factories::Baked m_unknownAssetLoaderFactory;

    World::Scene m_currentScene;
    SceneLoader* m_currentSceneLoader;
    Vector< SceneRequest > m_pendingLocalSceneRequests;
    Vector< SceneRequest > m_pendingRemoteSceneRequests;

    World::Coordinates m_coordinates;

    char* m_heightMap;
    char* m_collisionMap;

    User m_user;
    int m_positionRow;
    int m_positionCol;
    bool m_positionHidden;
    char m_glyphOffset;

    bool m_teleportRequested;
    Coordinates m_teleportCoordinates;
    int m_teleportRow;
    int m_teleportCol;

    PresenceLoader* m_currentPresenceLoader;
    Vector< ScenePresence > m_presences;
    Vector< PresenceRequest > m_pendingLocalPresenceRequests;
    Vector< PresenceRequest > m_pendingRemotePresenceRequests;

    Vector< PresenceRequest > m_pendingMyPresenceMoveRequests;

    int m_walkCycle;
    
    Timer* m_idleTimer;
    Timer* m_loadersUpdateTimer;
    Timer* m_moveRequestTimer;
    Timer* m_cursorTimer;

    int m_cursorGlyph;
    int m_cursorAttributes;

    bool m_sceneLocked;

    Vector< SceneItem >::const_iterator m_currentItem;
    bool m_itemCursorEnabled;

    Vector< String > m_programs;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_COMPOSITOR_H
