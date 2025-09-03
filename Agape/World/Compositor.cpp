#include "Compositor.h"

#include "Terminal.h"
#include "SceneLoaders/Factories/SceneLoadersFactory.h"
#include "SceneLoaders/SceneLoader.h"
#include "SceneLoaders/SceneRequest.h"
#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/AssetLoader.h"
#include "Assets/Asset.h"
#include "Assets/ANSIFile.h"

#include "Scene.h"
#include "SceneItem.h"

#include "Loggers/Logger.h"
#include "Utils/LiteStream.h"
#include "Utils/Tokeniser.h"

#include "Clocks/Clock.h"

#include "String.h"
#include "StringConstants.h"

#include <new>
#include "Allocator.h"
#include "Collections.h"

#include "PresenceLoaders/PresenceLoader.h"
#include "PresenceLoaders/Factories/PresenceLoadersFactory.h"
#include "World/ScenePresence.h"

#include "Audio/MIDIPlayer.h"

#include "World/WorldCoordinates.h"

#include "ProgramManager.h"

#include "Tuple.h"
#include "TupleRouter.h"

#include "Utils/LiteStream.h"

#include "Direction.h"

#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"

#include "Worldbook.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <cstdlib>

#include <algorithm>

#include "Warp.h"

#include "Height.h"

namespace
{
    const int idleTimeout( 1000 ); // ms
    const int loadersUpdatePeriod( 250 ); // ms
    const int moveRequestPeriod( 250 ); // ms
    const int cursorPeriod( 50 ); // ms

    const int maxQueuedRequests( 6 );

    const int textBorder( 1 );
    const int textAttributes( 0x0F );
} // Anonymous namespace

namespace Agape
{

namespace World
{

Compositor::Compositor( Terminal& terminal,
                        SceneLoaders::Factory& sceneLoaderFactory,
                        AssetLoaders::Factory& assetLoaderFactory,
                        PresenceLoaders::Factory& presenceLoaderFactory,
                        Worldbook& worldbook,
                        Linda2::TupleRouter& tupleRouter,
                        Carlo::ProgramManager& programManager,
                        AssetLoaders::Factory& programAssetLoaderFactory,
                        Timers::Factory& timerFactory,
                        Clock& clock,
                        Audio::MIDIPlayer& midiPlayer ) :
  m_terminal( terminal ),
  m_sceneLoaderFactory( sceneLoaderFactory ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_presenceLoaderFactory( presenceLoaderFactory ),
  m_worldbook( worldbook ),
  m_tupleRouter( tupleRouter ),
  m_programManager( programManager ),
  m_programAssetLoaderFactory( programAssetLoaderFactory ),
  m_clock( clock ),
  m_midiPlayer( midiPlayer ),
  m_currentSceneLoader( nullptr ),
#ifdef NOTHROW
  m_heightMap( new ( std::nothrow ) char[terminal.width()*terminal.height()] ),
  m_collisionMap( new ( std::nothrow ) char[terminal.width()*terminal.height()] ),
#else
  m_heightMap( new char[terminal.width()*terminal.height()] ),
  m_collisionMap( new char[terminal.width()*terminal.height()] ),
#endif
  m_positionRow( 0 ),
  m_positionCol( 0 ),
  m_positionHidden( false ),
  m_glyphOffset( 0 ),
  m_teleportRequested( false ),
  m_teleportRow( -1 ),
  m_teleportCol( -1 ),
  m_currentPresenceLoader( nullptr ),
  m_walkCycle( 0 ),
  m_idleTimer( timerFactory.makeTimer() ),
  m_loadersUpdateTimer( timerFactory.makeTimer() ),
  m_moveRequestTimer( timerFactory.makeTimer() ),
  m_cursorTimer( timerFactory.makeTimer() ),
  m_cursorGlyph( 0x1e ),
  m_cursorAttributes( 0x0e ),
  m_sceneLocked( false ),
  m_currentItem( m_currentScene.m_sceneItems.end() ),
  m_itemCursorEnabled( false )
{
    ::memset( m_heightMap, 0, terminal.width()*terminal.height() );
    ::memset( m_collisionMap, -1, terminal.width()*terminal.height() );

    m_pendingLocalSceneRequests.reserve( maxQueuedRequests );
    m_pendingRemoteSceneRequests.reserve( maxQueuedRequests );
    m_pendingLocalPresenceRequests.reserve( maxQueuedRequests );
    m_pendingRemotePresenceRequests.reserve( maxQueuedRequests );
    m_pendingMyPresenceMoveRequests.reserve( 1 );
}

Compositor::~Compositor()
{
    delete( m_currentSceneLoader );
    delete( m_currentPresenceLoader );

    delete[]( m_heightMap );
    delete[]( m_collisionMap );

    delete( m_idleTimer );
    delete( m_loadersUpdateTimer );
    delete( m_moveRequestTimer );
    delete( m_cursorTimer );
}

int Compositor::height() const
{
    return m_terminal.height();
}

int Compositor::width() const
{
    return m_terminal.width();
}

void Compositor::render( const Coordinates& newCoordinates )
{
    Warp w1( "Render scene" );

    m_terminal.printFormatted( "Chill...",
                               m_terminal.height() / 2,
                               0,
                               1,
                               Terminal::noMaxWidth,
                               Terminal::hCentre,
                               Terminal::noVCentre,
                               0x0F );

    //LOG_DEBUG( "Compositor: Loading scene" );
    Warp w2( "Depart" );
    depart();
    w2.report();

    Warp w3( "Load scene" );
    m_currentSceneLoader = m_sceneLoaderFactory.makeLoader( newCoordinates );
    m_currentSceneLoader->load( m_currentScene ); // Continue on failure.
    w3.report();

    Warp w4( "Load presences" );
    m_currentPresenceLoader = m_presenceLoaderFactory.makeLoader( newCoordinates );
    m_currentPresenceLoader->load( m_presences ); // Continue on failure.
    w4.report();

    Vector< ScenePresence >::const_iterator presenceIter;
    for( presenceIter = m_presences.begin(); presenceIter != m_presences.end(); ++presenceIter )
    {
        if( presenceIter->m_user.m_snowflake != m_user.m_snowflake )
        {
            m_terminal.createCursor( presenceIter->m_user.m_snowflake, presenceIter->m_row, presenceIter->m_col, presenceIter->m_user.m_glyph, presenceIter->m_user.m_attributes, Terminal::avatarCharset );
        }
    }

    m_currentItem = m_currentScene.m_sceneItems.end();
    if( !m_currentScene.m_sceneItems.empty() )
    {
        m_currentItem--; // Select last item.
    }

    // Note: Compositor uses its own copy of coordinates, not a reference to the
    // shared object used by WalkStrategy etc. This is so Compositor can
    // remember the previous coordinates to depart() before renreding the scene
    // at the new coordinates.
    m_coordinates = newCoordinates;

    m_teleportRequested = false;
    m_teleportRow = -1;
    m_teleportCol = -1;

    Warp w5( "Paint scene" );
    render();
    w5.report();

    Warp w6( "Load actions" );
    performActionOnAll( loadAction );
    w6.report();

    w1.report();
}

void Compositor::render()
{
    //LOG_DEBUG( "Compositor: Resetting height map" );
    ::memset( m_heightMap, 0, width() * height() );

    //LOG_DEBUG( "Compositor: Resetting collision map" );
    ::memset( m_collisionMap, -1, width() * height() );

    // Reload programs. FIXME: Does this belong here, or only in the above
    // when we're actually changing scenes? If we do change this, we need to
    // change the cache invalidation stuff, as that expects render() to
    // reload all programs.
    Vector< String >::const_iterator it( m_programs.begin() );
    for( ; it != m_programs.end(); ++it )
    {
        m_programManager.unload( *it );
    }
    m_programs.clear();

    m_terminal.deleteAllSprites();

    //LOG_DEBUG( "Compositor: Tiling background" );
    tileBackground();
    
    Vector< SceneItem >::iterator iter;
    int itemIdx( 0 );
    for( iter = m_currentScene.m_sceneItems.begin(); iter != m_currentScene.m_sceneItems.end(); ++iter )
    {
        SceneItem& thisSceneItem( *iter );
        //LOG_DEBUG( String( "Compositor: Rendering " ) + thisSceneItem.m_assetName );

        Warp w1( "Open asset" );

        AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, thisSceneItem.assetName() ) );
        if( !assetLoader->open() )
        {
            delete( assetLoader );
            assetLoader = m_unknownAssetLoaderFactory.makeLoader( Coordinates(), "unknown" );
            assetLoader->open();
        }

        w1.report();
        Warp w2( "Draw asset" );

        if( thisSceneItem.assetName()[0] == '*' )
        {
            Asset asset( *assetLoader );
            m_terminal.consumeNext( thisSceneItem.row(), thisSceneItem.col() );
            m_terminal.consumeGraphicalAsset( asset, asset.size() );
            // FIXME: Set height map for entire asset?
        }
        else
        {
            Assets::ANSIFile ansiFile( *assetLoader );

            bool blit( false );
            bool sprite( false );
            bool animate( false );
            int frames( 0 );
            String templateName;
            if( ansiFile.hasSAUCE() )
            {
                ansiFile.getAssetFlags( blit, sprite, animate, frames, templateName );

                // Load a template program for this object if a template is specified in SAUCE.
                loadTemplateProgram( thisSceneItem.snowflake(), templateName );
            }

            if( blit || sprite || animate )
            {
                createSprite( thisSceneItem.snowflake(),
                              thisSceneItem.assetName(),
                              thisSceneItem.row(),
                              thisSceneItem.col() );
            }
            else
            {
                m_terminal.consumeNext( thisSceneItem.row(), thisSceneItem.col() );
                m_terminal.consumeAsset( ansiFile,
                                         0, // Offset zero
                                         ansiFile.dataSize(),
                                         ansiFile.width(),
                                         thisSceneItem.col(),
                                         Terminal::noMaxRow,
                                         Terminal::scrollLock,
                                         Terminal::whitespaceTransparency | Terminal::ANSI,
                                         m_collisionMap,
                                         itemIdx );
                if( ansiFile.hasSAUCE() )
                {
                    ansiFile.getHeights( m_heightMap,
                                         m_terminal.height(),
                                         m_terminal.width(),
                                         thisSceneItem.row(),
                                         thisSceneItem.col() );
                }
            }

            // The scene item will store the dimensions of the ANSI file at
            // creation time, but if the ANSI file is resized these dimensions
            // may be wrong. Patch the scene item with the correct
            // dimensions here.
            thisSceneItem.setDimensions( animate ? ansiFile.height() / frames : ansiFile.height(),
                                         ansiFile.width() );
        }

        delete( assetLoader );

        w2.report();
        Warp w3( "Load linked" );

        if( thisSceneItem.flags() & SceneItem::linkedProgram )
        {
            // Try to load a program for this object based on its snowflake.
            loadLinkedProgram( thisSceneItem.snowflake() ); 
        }

        if( thisSceneItem.flags() & SceneItem::linkedText )
        {
            AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, thisSceneItem.snowflake() + "_txt" ) );

            if( assetLoader->open() )
            {
                Asset asset( *assetLoader );
                String text( asset.size(), '\0' );
                if( asset.readAll( &text[0], 0, asset.size() ) == asset.size() )
                {
                    // Private function always draws and doesn't check m_sceneLocked.
                    _drawTextItem( thisSceneItem, text );
                }
            }

            delete( assetLoader );
        }

        w3.report();

        m_midiPlayer.run(); // Keep enqueuing music

        ++itemIdx;
    }

    // May include loading a program based on object's action field.
    performActionOnAll( renderAction );

    // Try to load whole-world program, if any.
    if( m_programManager.load( m_programAssetLoaderFactory,
                               m_coordinates,
                               _World ) )
    {
        m_programs.push_back( _World );
    }

    // Try to load whole-scene program, if any.
    LiteStream stream;
    stream << _scene_ << m_coordinates.m_x << "_" << m_coordinates.m_y;
    if( m_programManager.load( m_programAssetLoaderFactory,
                               m_coordinates,
                               stream.str() ) )
    {
        m_programs.push_back( stream.str() );
    }

    m_terminal.redrawCursors();
}

void Compositor::depart()
{
    Vector< String >::const_iterator it( m_programs.begin() );
    for( ; it != m_programs.end(); ++it )
    {
        m_programManager.unload( *it );
    }
    m_programs.clear();

    m_terminal.deleteAllSprites();

    if( m_currentSceneLoader != nullptr )
    {
        delete( m_currentSceneLoader ); // If Linda2, unsubscribes from scene updates at these coordinates.
        m_currentSceneLoader = nullptr;
        
        m_pendingLocalSceneRequests.clear();
        m_pendingRemoteSceneRequests.clear();
    }

    if( m_currentPresenceLoader != nullptr )
    {
        m_terminal.deleteAllCursors();
        m_presences.clear();

        // Announce our departure
        // FIXME: Copypasta. Break into functions.
        ScenePresence scenePresence;
        scenePresence.m_user = m_user;
        scenePresence.m_coordinates = m_coordinates; // Old coordinates
        scenePresence.m_row = m_positionRow;
        scenePresence.m_col = m_positionCol;

        PresenceRequest presenceRequest( PresenceRequest::depart, scenePresence, m_coordinates );

        Vector< PresenceRequest > presenceRequests;
        presenceRequests.push_back( presenceRequest );
        m_currentPresenceLoader->request( presenceRequests ); // Ignore failures.

        updatePresences( presenceRequests ); // Perform local update immediately.

        delete( m_currentPresenceLoader ); // If Linda2, unsubscribes from presence updates at these coordinates.
        m_currentPresenceLoader = nullptr;

        m_pendingLocalPresenceRequests.clear();
        m_pendingRemotePresenceRequests.clear();
        m_pendingMyPresenceMoveRequests.clear();
    }
}

void Compositor::setUser( const User& user )
{
    m_user = user;
}

void Compositor::setPosition( int row, int col, enum Direction::_Direction direction )
{
    m_positionRow = row;
    m_positionCol = col;

    char height( m_heightMap[ ( width() * row ) + col ] );

    if( height != foreground )
    {
        m_positionHidden = false;

        ScenePresence scenePresence;
        scenePresence.m_user = m_user;
        scenePresence.m_coordinates = m_coordinates;
        scenePresence.m_row = m_positionRow;
        scenePresence.m_col = m_positionCol;

        PresenceRequest presenceRequest( PresenceRequest::arrive, scenePresence, m_coordinates, direction );

        Vector< PresenceRequest > presenceRequests;
        presenceRequests.push_back( presenceRequest );
        m_currentPresenceLoader->request( presenceRequests ); // Ignore failures.

        updatePresences( presenceRequests ); // Perform local update immediately.
    }
    else
    {
        m_positionHidden = true;
    }
}

void Compositor::setPositionToPrevious()
{
    setPosition( m_positionRow, m_positionCol );
}

int Compositor::positionRow() const
{
    return m_positionRow;
}

int Compositor::positionCol() const
{
    return m_positionCol;
}

void Compositor::walk( enum Direction::_Direction direction, bool& atEdge )
{   
    atEdge = false;
    if( direction == Direction::up )
    {
        if( m_positionRow > 0 )
        {
            tryWalk( direction, m_positionRow - 1, m_positionCol, 1, 2 );
        }
        else
        {
            atEdge = true;
        }
    }
    else if( direction == Direction::down )
    {
        if( m_positionRow < ( height() - 1 ) )
        {
            tryWalk( direction, m_positionRow + 1, m_positionCol, 3, 4 );
        }
        else
        {
            atEdge = true;
        }
    }
    else if( direction == Direction::left )
    {
        if( m_positionCol > 0 )
        {
            tryWalk( direction, m_positionRow, m_positionCol - 1, 5, 6 );
        }
        else
        {
            atEdge = true;
        }
    }
    else if( direction == Direction::right )
    {
        if( m_positionCol < ( width() - 1 ) )
        {
            tryWalk( direction, m_positionRow, m_positionCol + 1, 7, 8 );
        }
        else
        {
            atEdge = true;
        }
    }
}

void Compositor::createSprite( const String& name, const String& assetName, int row, int col )
{
    AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, assetName ) );
    if( !assetLoader->open() )
    {
        delete( assetLoader );
        assetLoader = m_unknownAssetLoaderFactory.makeLoader( Coordinates(), "unknown" );
        assetLoader->open();
    }

    Assets::ANSIFile ansiFile( *assetLoader );

    bool blit( false );
    bool spriteflag( false );
    bool animate( false );
    int frames( 0 );
    String templateName;
    if( ansiFile.hasSAUCE() )
    {
        ansiFile.getAssetFlags( blit, spriteflag, animate, frames, templateName );
    }

    if( animate && ( ( frames < 2 ) || ( ( ansiFile.height() % frames ) != 0 ) ) ) return;

    m_terminal.createSprite( name,
                             assetName,
                             ansiFile,
                             ansiFile.dataSize(),
                             row,
                             col,
                             animate ? ansiFile.height() / frames : ansiFile.height(),
                             ansiFile.width(),
                             Terminal::whitespaceTransparency | ( blit ? Terminal::blit : 0 ) | Terminal::ANSI,
                             animate ? frames : 1 );

    if( ansiFile.hasSAUCE() )
    {
        char* spriteMap;
        if( m_terminal.spriteMap( name, spriteMap ) )
        {
            ansiFile.getHeights( spriteMap,
                                 animate ? ansiFile.height() / frames : ansiFile.height(),
                                 ansiFile.width(),
                                 0,
                                 0 );
        }
    }

    delete( assetLoader );
}

bool Compositor::isSprite( const String& name ) const
{
    return m_terminal.isSprite( name );
}

void Compositor::moveSprite( const String& name, int row, int col )
{
    m_terminal.moveSprite( name, row, col );
}

bool Compositor::spriteData( const String& name, String& assetName, int& row, int& col, int& height, int& width )
{
    return m_terminal.spriteData( name, assetName, row, col, height, width );
}

void Compositor::deleteSprite( const String& name )
{
    m_terminal.deleteSprite( name );
}

void Compositor::spriteToScene( const String& name, const String& action, bool immediate )
{
    // FIXME: We should be passed a decorator to do this,
    // so Compositor doesn't need a clock and to know
    // user details. These could also just be passed in via
    // function arguments?

    String assetName;
    int row( 0 );
    int col( 0 );
    int height( 0 );
    int width( 0 );
    if( m_terminal.spriteData( name, assetName, row, col, height, width ) )
    {
        SceneItem newItem( assetName,
                           row,
                           col,
                           height,
                           width,
                           m_user.m_snowflake,
                           action,
                           m_clock );

        LOG_DEBUG( "Requesting scene create" );
        SceneRequest request( SceneRequest::create, newItem, m_coordinates );
        Vector< SceneRequest > requests;
        requests.push_back( request );
        m_currentSceneLoader->request( requests );

        if( immediate )
        {
            updateScene( requests );
        }
        else if( m_pendingLocalSceneRequests.size() < maxQueuedRequests )
        {
            m_pendingLocalSceneRequests.push_back( request );
        }

        m_terminal.deleteSprite( name );
    }
}

void Compositor::createItem( const String& name, const String& action, int row, int col, bool immediate )
{
    if( m_currentSceneLoader )
    {
        // Need to open asset to find height and width.
        int height( 0 );
        int width( 0 );
        getAssetDimensions( name, height, width );

        // Create request.
        SceneItem newItem( name,
                           row,
                           col,
                           height,
                           width,
                           m_user.m_snowflake,
                           action,
                           m_clock );

        LOG_DEBUG( "Requesting scene create" );
        SceneRequest request( SceneRequest::create, newItem, m_coordinates );
        Vector< SceneRequest > requests;
        requests.push_back( request );
        m_currentSceneLoader->request( requests );

        if( immediate )
        {
            updateScene( requests );
        }
        else if( m_pendingLocalSceneRequests.size() < maxQueuedRequests )
        {
            m_pendingLocalSceneRequests.push_back( request );
        }
    }
}

void Compositor::moveItem( const String& snowflake, int row, int col, enum Direction::_Direction direction, bool keyboard, bool immediate )
{
    const SceneItem* sceneItem( nullptr );
    if( !snowflake.empty() )
    {
        sceneItem = findItemBySnowflake( snowflake );
    }
    else if( m_currentItem != m_currentScene.m_sceneItems.end() )
    {
        sceneItem = &( *m_currentItem );
    }

    if( sceneItem )
    {
        SceneRequest request( SceneRequest::update, *sceneItem, m_coordinates, direction, keyboard );

        request.m_sceneItem.setRow( row );
        request.m_sceneItem.setCol( col );
        request.m_sceneItem.touch();

        LOG_DEBUG( "Requesting scene update" );
        if( immediate )
        {
            Vector< SceneRequest > requests;
            requests.push_back( request );
            if( m_currentSceneLoader ) m_currentSceneLoader->request( requests );
            updateScene( requests );
        }
        else if( ( m_pendingLocalSceneRequests.size() < maxQueuedRequests ) &&
                 ( m_pendingRemoteSceneRequests.size() < maxQueuedRequests ) )
        {
            // Look for existing move request for this scene item.
            Vector< SceneRequest >::iterator it( m_pendingRemoteSceneRequests.begin() );
            for( ; it != m_pendingRemoteSceneRequests.end(); ++it )
            {
                if( it->m_sceneItem == *sceneItem )
                {
                    *it = request;
                    break;
                }
            }

            // Add new move request if no existing.
            if( ( it == m_pendingRemoteSceneRequests.end() ) &&
                ( m_pendingRemoteSceneRequests.size() < maxQueuedRequests ) )
            {
                m_pendingRemoteSceneRequests.push_back( request );
            }

            m_pendingLocalSceneRequests.push_back( request );
        }
    }
}

void Compositor::moveItem( const String& snowflake, enum Direction::_Direction direction )
{
    const SceneItem* sceneItem( nullptr );
    if( !snowflake.empty() )
    {
        sceneItem = findItemBySnowflake( snowflake );
    }
    else if( m_currentItem != m_currentScene.m_sceneItems.end() )
    {
        sceneItem = &( *m_currentItem );
    }

    if( sceneItem )
    {
        switch( direction )
        {
        case Direction::left:
            if( ( sceneItem->col() + sceneItem->width() - 1 ) > 0 )
            {
                moveItem( snowflake, sceneItem->row(), sceneItem->col() - 1, direction );
            }
            else
            {
                transportItem( snowflake,
                               m_coordinates.m_x - 1,
                               m_coordinates.m_y,
                               sceneItem->row(),
                               ( width() - sceneItem->width() ) );
            }
            break;
        case Direction::right:
            if( sceneItem->col() < ( width() - 1 ) )
            {
                moveItem( snowflake, sceneItem->row(), sceneItem->col() + 1, direction );
            }
            else
            {
                transportItem( snowflake,
                               m_coordinates.m_x + 1,
                               m_coordinates.m_y,
                               sceneItem->row(),
                               0 );
            }
            break;
        case Direction::up:
            if( ( sceneItem->row() + sceneItem->height() - 1 ) > 0 )
            {
                moveItem( snowflake, sceneItem->row() - 1, sceneItem->col(), direction );
            }
            else
            {
                transportItem( snowflake,
                               m_coordinates.m_x,
                               m_coordinates.m_y + 1,
                               ( height() - sceneItem->height() ),
                               sceneItem->col() );
            }
            break;
        case Direction::down:
            if( sceneItem->row() < ( height() - 1 ) )
            {
                moveItem( snowflake, sceneItem->row() + 1, sceneItem->col(), direction );
            }
            else
            {
                transportItem( snowflake,
                               m_coordinates.m_x,
                               m_coordinates.m_y - 1,
                               0,
                               sceneItem->col() );
            }
            break;
        default:
            break;
        }
    }
}

void Compositor::updateItem( const String& snowflake, const String& name, const String& action, bool immediate, bool haveAction )
{
    if( m_currentSceneLoader )
    {
        const SceneItem* sceneItem( nullptr );
        if( !snowflake.empty() )
        {
            sceneItem = findItemBySnowflake( snowflake );
        }
        else if( m_currentItem != m_currentScene.m_sceneItems.end() )
        {
            sceneItem = &( *m_currentItem );
        }

        if( sceneItem )
        {
            SceneRequest request( SceneRequest::update, *sceneItem, m_coordinates );

            // FIXME: Can other fields be updated?
            // FIXME: Have seperate modification and creation times?
            request.m_sceneItem.setAssetName( name );
            if( haveAction ) request.m_sceneItem.setAction( action );

            int height( 0 );
            int width( 0 );
            getAssetDimensions( name, height, width );
            request.m_sceneItem.setDimensions( height, width );

            request.m_sceneItem.touch();

            LOG_DEBUG( "Requesting scene update" );
            Vector< SceneRequest > requests;
            requests.push_back( request );
            m_currentSceneLoader->request( requests );

            if( immediate )
            {
                updateScene( requests );
            }
            else if( m_pendingLocalSceneRequests.size() < maxQueuedRequests )
            {
                m_pendingLocalSceneRequests.push_back( request );
            }
        }
    }
}

void Compositor::setItemFlags( const String& snowflake, enum SceneItem::Flags flags, bool immediate )
{
    if( m_currentSceneLoader )
    {
        const SceneItem* sceneItem( nullptr );
        if( !snowflake.empty() )
        {
            sceneItem = findItemBySnowflake( snowflake );
        }
        else if( m_currentItem != m_currentScene.m_sceneItems.end() )
        {
            sceneItem = &( *m_currentItem );
        }

        if( sceneItem )
        {
            SceneRequest request( SceneRequest::update, *sceneItem, m_coordinates );

            request.m_sceneItem.setFlags( flags );
            request.m_sceneItem.touch();

            LOG_DEBUG( "Requesting scene update" );
            Vector< SceneRequest > requests;
            requests.push_back( request );
            m_currentSceneLoader->request( requests );

            if( immediate )
            {
                updateScene( requests );
            }
            else if( m_pendingLocalSceneRequests.size() < maxQueuedRequests )
            {
                m_pendingLocalSceneRequests.push_back( request );
            }
        }
    }
}

void Compositor::deleteItem( const String& snowflake, bool immediate )
{
    if( m_currentSceneLoader )
    {
        const SceneItem* sceneItem( nullptr );
        if( !snowflake.empty() )
        {
            sceneItem = findItemBySnowflake( snowflake );
        }
        else if( m_currentItem != m_currentScene.m_sceneItems.end() )
        {
            sceneItem = &( *m_currentItem );
        }

        if( sceneItem )
        {
            // Unload linked program or template program instance, if any.
            Vector< String >::const_iterator programsIt( m_programs.begin() );
            for( ; programsIt != m_programs.end(); ++programsIt )
            {
                if( *programsIt == sceneItem->snowflake() )
                {
                    m_programManager.unload( *programsIt );
                    break;
                }
            }

            // Delete linked program, linked text and attributes.
            if( sceneItem->flags() & SceneItem::linkedProgram )
            {
                m_programManager.erase( m_programAssetLoaderFactory, m_coordinates, sceneItem->snowflake() );
            }

            if( sceneItem->flags() & SceneItem::linkedText )
            {
                AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, sceneItem->snowflake() + "_txt" ) );
                assetLoader->erase();
                delete( assetLoader );
            }

            m_currentSceneLoader->deleteSceneItemAttributes( snowflake );

            // Send remote removal request and queue/perform local removal.
            SceneRequest request( SceneRequest::remove, *sceneItem, m_coordinates );

            request.m_sceneItem.touch();

            LOG_DEBUG( "Requesting scene remove" );
            Vector< SceneRequest > requests;
            requests.push_back( request );
            m_currentSceneLoader->request( requests );

            if( immediate )
            {
                updateScene( requests );
            }
            else if( m_pendingLocalSceneRequests.size() < maxQueuedRequests )
            {
                m_pendingLocalSceneRequests.push_back( request );
            }

            // Look for existing move requests and remove (as no longer valid).
            Vector< SceneRequest >::iterator sceneIt( m_pendingRemoteSceneRequests.begin() );
            for( ; sceneIt != m_pendingRemoteSceneRequests.end(); ++sceneIt )
            {
                if( sceneIt->m_sceneItem == *sceneItem )
                {
                    m_pendingRemoteSceneRequests.erase( sceneIt );
                    break;
                }
            }
        }
    }
}

void Compositor::transportItem( const String& snowflake, int x, int y, int row, int col )
{
    if( m_currentSceneLoader )
    {
        const SceneItem* sceneItem( nullptr );
        if( !snowflake.empty() )
        {
            sceneItem = findItemBySnowflake( snowflake );
        }
        else if( m_currentItem != m_currentScene.m_sceneItems.end() )
        {
            sceneItem = &( *m_currentItem );
        }

        if( sceneItem )
        {
            // NOTE: We currently don't support transporting items to other
            // worlds here - if we do, the scene item will need to be
            // re-encrypted with the destination world key, and the server will
            // need to check that we have permission to write to the
            // new world!
            SceneItem newSceneItem( *sceneItem );
            if( row != -1 ) newSceneItem.setRow( row );
            if( col != -1 ) newSceneItem.setCol( col );
            SceneRequest request( newSceneItem, m_coordinates, Coordinates( m_coordinates.m_worldID, x, y ) );
            Vector< SceneRequest > sceneRequests;
            sceneRequests.push_back( request );
            m_currentSceneLoader->request( sceneRequests );

            if( m_pendingLocalSceneRequests.size() < maxQueuedRequests )
            {
                m_pendingLocalSceneRequests.push_back( request );
            }

            // Look for existing move requests and remove (as no longer valid).
            Vector< SceneRequest >::iterator it( m_pendingRemoteSceneRequests.begin() );
            for( ; it != m_pendingRemoteSceneRequests.end(); ++it )
            {
                if( it->m_sceneItem == *sceneItem )
                {
                    m_pendingRemoteSceneRequests.erase( it );
                    break;
                }
            }
        }
    }
}

void Compositor::raiseItem( const String& snowflake, bool immediate )
{
    if( m_currentSceneLoader )
    {
        const SceneItem* sceneItem( nullptr );
        if( !snowflake.empty() )
        {
            sceneItem = findItemBySnowflake( snowflake );
        }
        else if( m_currentItem != m_currentScene.m_sceneItems.end() )
        {
            sceneItem = &( *m_currentItem );
        }

        if( sceneItem )
        {
            SceneRequest request( SceneRequest::raise, *sceneItem, m_coordinates );
            request.m_sceneItem.touch();

            LOG_DEBUG( "Requesting scene update" );
            Vector< SceneRequest > requests;
            requests.push_back( request );
            m_currentSceneLoader->request( requests );

            if( immediate )
            {
                updateScene( requests );
            }
            else if( m_pendingLocalSceneRequests.size() < maxQueuedRequests )
            {
                m_pendingLocalSceneRequests.push_back( request );
            }
        }
    }
}

void Compositor::lowerItem( const String& snowflake, bool immediate )
{
    if( m_currentSceneLoader )
    {
        const SceneItem* sceneItem( nullptr );
        if( !snowflake.empty() )
        {
            sceneItem = findItemBySnowflake( snowflake );
        }
        else if( m_currentItem != m_currentScene.m_sceneItems.end() )
        {
            sceneItem = &( *m_currentItem );
        }

        if( sceneItem )
        {
            SceneRequest request( SceneRequest::lower, *sceneItem, m_coordinates );
            request.m_sceneItem.touch();

            LOG_DEBUG( "Requesting scene update" );
            Vector< SceneRequest > requests;
            requests.push_back( request );
            m_currentSceneLoader->request( requests );

            if( immediate )
            {
                updateScene( requests );
            }
            else if( m_pendingLocalSceneRequests.size() < maxQueuedRequests )
            {
                m_pendingLocalSceneRequests.push_back( request );
            }
        }
    }
}

void Compositor::moveCurrentItem( int row, int col, enum Direction::_Direction direction, bool keyboard, bool immediate )
{
    if( m_currentItem != m_currentScene.m_sceneItems.end() )
    {
        moveItem( m_currentItem->snowflake(), row, col, direction, keyboard, immediate );
    }
}

void Compositor::updateCurrentItem( const String& name, const String& action, bool immediate, bool haveAction )
{
    if( m_currentItem != m_currentScene.m_sceneItems.end() )
    {
        updateItem( m_currentItem->snowflake(), name, action, immediate, haveAction );
    }
}

void Compositor::deleteCurrentItem( bool immediate )
{
    if( m_currentItem != m_currentScene.m_sceneItems.end() )
    {
        deleteItem( m_currentItem->snowflake(), immediate );
    }
}

void Compositor::raiseCurrentItem( bool immediate )
{
    if( m_currentItem != m_currentScene.m_sceneItems.end() )
    {
        raiseItem( m_currentItem->snowflake(), immediate );
    }
}

void Compositor::lowerCurrentItem( bool immediate )
{
    if( m_currentItem != m_currentScene.m_sceneItems.end() )
    {
        lowerItem( m_currentItem->snowflake(), immediate );
    }
}

void Compositor::addTextCurrentItem()
{
    if( m_currentItem != m_currentScene.m_sceneItems.end() )
    {
        bool didRender( false );
        notifyInvalidated( m_currentItem->snowflake() + "_txt",
                           _asset,
                           didRender );
        setItemFlags( m_currentItem->snowflake(),
                      (SceneItem::Flags)( m_currentItem->flags() | SceneItem::linkedText ),
                      true ); // true = update immediately.
    }
}

void Compositor::deleteTextCurrentItem()
{
    if( m_currentItem != m_currentScene.m_sceneItems.end() )
    {
        AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, m_currentItem->snowflake() + "_txt" ) );
        assetLoader->erase();
        delete( assetLoader );

        bool didRender( false );
        notifyInvalidated( m_currentItem->snowflake() + "_txt",
                           _asset,
                           didRender );
        setItemFlags( m_currentItem->snowflake(),
                      (SceneItem::Flags)( m_currentItem->flags() & ~SceneItem::linkedText ),
                      true ); // true = update immediately.
    }
}

void Compositor::drawTextItem( const SceneItem& sceneItem, const String& text, bool clearFirst )
{
    if( m_sceneLocked ) return;

    _drawTextItem( sceneItem, text, clearFirst );
}

const ScenePresence* Compositor::findPresenceBySnowflake( const String& snowflake ) const
{
    Vector< ScenePresence >::const_iterator it( m_presences.begin() );
    for( ; it != m_presences.end(); ++it )
    {
        if( it->m_user.m_snowflake == snowflake )
        {
            return &( *it );
        }
    }

    return nullptr;
}

void Compositor::createPerson( const String& name, int glyph, int attributes, int row, int col )
{
    if( m_currentPresenceLoader )
    {
        User user;
        user.m_name = name;
        user.m_glyph = glyph;
        user.m_attributes = attributes;

        ScenePresence scenePresence;
        scenePresence.m_user = user;
        scenePresence.m_coordinates = m_coordinates;
        scenePresence.m_row = row;
        scenePresence.m_col = col;

        PresenceRequest request( PresenceRequest::arrive, scenePresence, m_coordinates );

        Vector< PresenceRequest > presenceRequests;
        presenceRequests.push_back( request );
        m_currentPresenceLoader->request( presenceRequests );

        if( m_pendingLocalPresenceRequests.size() < maxQueuedRequests )
        {
            m_pendingLocalPresenceRequests.push_back( request );
        }
    }
}

void Compositor::movePerson( const String& snowflake, int row, int col, enum Direction::_Direction direction )
{
    const ScenePresence* presence( findPresenceBySnowflake( snowflake ) );

    if( presence )
    {
        PresenceRequest request( PresenceRequest::move,
                                    *presence,
                                    m_coordinates,
                                    direction );
        
        request.m_scenePresence.m_row = row;
        request.m_scenePresence.m_col = col;

        if( ( m_pendingLocalPresenceRequests.size() < maxQueuedRequests ) &&
            ( m_pendingRemotePresenceRequests.size() < maxQueuedRequests ) )
        {
            if( snowflake == m_user.m_snowflake )
            {
                // Set as our own pending move request. This may be elided if
                // we move locally again before we're due to send a request
                // to the server.
                m_pendingMyPresenceMoveRequests.clear();
                m_pendingMyPresenceMoveRequests.push_back( request );
            }
            else
            {
                // Look for existing move request for this presence.
                Vector< PresenceRequest >::iterator it( m_pendingRemotePresenceRequests.begin() );
                for( ; it != m_pendingRemotePresenceRequests.end(); ++it )
                {
                    if( it->m_scenePresence.m_user.m_snowflake == snowflake )
                    {
                        // Elide any existing.
                        *it = request;
                        break;
                    }
                }

                // Add new move request if no existing.
                if( it == m_pendingRemotePresenceRequests.end() )
                {
                    m_pendingRemotePresenceRequests.push_back( request );
                }
            }

            m_pendingLocalPresenceRequests.push_back( request );
        }
    }
}

void Compositor::movePerson( const String& snowflake, enum Direction::_Direction direction )
{
    const ScenePresence* presence( findPresenceBySnowflake( snowflake ) );

    if( presence )
    {
        bool sendPresenceRequest( true );
        int moveRow( presence->m_row );
        int moveCol( presence->m_col );

        switch( direction )
        {
        case Direction::left:
            if( presence->m_col > 0 )
            {
                moveCol = presence->m_col - 1;
            }
            else if( snowflake == m_user.m_snowflake )
            {
                m_teleportCoordinates = m_coordinates;
                --m_teleportCoordinates.m_x;
                m_teleportRow = m_positionRow;
                m_teleportCol = width() - 1;
                m_teleportRequested = true;
                sendPresenceRequest = false;
            }
            break;
        case Direction::right:
            if( presence->m_col < ( width() - 1 ) )
            {
                moveCol = presence->m_col + 1;
            }
            else if( snowflake == m_user.m_snowflake )
            {
                m_teleportCoordinates = m_coordinates;
                ++m_teleportCoordinates.m_x;
                m_teleportRow = m_positionRow;
                m_teleportCol = 0;
                m_teleportRequested = true;
                sendPresenceRequest = false;
            }
            break;
        case Direction::up:
            if( presence->m_row > 0 )
            {
                moveRow = presence->m_row - 1;
            }
            else if( snowflake == m_user.m_snowflake )
            {
                m_teleportCoordinates = m_coordinates;
                --m_teleportCoordinates.m_y;
                m_teleportRow = height() - 1;
                m_teleportCol = m_positionCol;
                m_teleportRequested = true;
                sendPresenceRequest = false;
            }
            break;
        case Direction::down:
            if( presence->m_row < ( height() - 1 ) )
            {
                moveRow = presence->m_row + 1;
            }
            else if( snowflake == m_user.m_snowflake )
            {
                m_teleportCoordinates = m_coordinates;
                ++m_teleportCoordinates.m_y;
                m_teleportRow = 0;
                m_teleportCol = m_positionCol;
                m_teleportRequested = true;
                sendPresenceRequest = false;
            }
            break;
        default:
            break;
        }

        if( sendPresenceRequest )
        {
            movePerson( snowflake, moveRow, moveCol, direction );
        }
    }
}

void Compositor::deletePerson( const String& snowflake )
{
    if( m_currentPresenceLoader )
    {
        const ScenePresence* presence( findPresenceBySnowflake( snowflake ) );

        if( presence )
        {
            PresenceRequest request( PresenceRequest::depart,
                                    *presence,
                                    m_coordinates );
            
            Vector< PresenceRequest > requests;
            requests.push_back( request );
            m_currentPresenceLoader->request( requests );

            if( m_pendingLocalPresenceRequests.size() < maxQueuedRequests )
            {
                m_pendingLocalPresenceRequests.push_back( request );
            }

            // Look for existing move requests and remove (as no longer valid).
            if( snowflake == m_user.m_snowflake )
            {
                m_pendingMyPresenceMoveRequests.clear();
            }
            else
            {
                Vector< PresenceRequest >::iterator it( m_pendingRemotePresenceRequests.begin() );
                for( ; it != m_pendingRemotePresenceRequests.end(); ++it )
                {
                    if( it->m_scenePresence.m_user.m_snowflake == snowflake )
                    {
                        m_pendingRemotePresenceRequests.erase( it );
                        break;
                    }
                }
            }
        }
    }
}

void Compositor::setCursorVariant( int variant )
{
    m_terminal.setCursorVariant( variant );
}

const SceneItem* Compositor::selectLast()
{
    if( m_currentItem != m_currentScene.m_sceneItems.end() )
    {
        if( !m_itemCursorEnabled )
        {
            m_cursorGlyph = 0x1e;
            m_cursorAttributes = 0x0e;
            m_terminal.createCursor( "Edit", m_currentItem->row(), m_currentItem->col(), m_cursorGlyph, m_cursorAttributes );
            m_itemCursorEnabled = true;
            m_cursorTimer->reset();
        }
        else
        {
            m_terminal.moveCursor( "Edit", m_currentItem->row(), m_currentItem->col() );
        }
        
        return &( *m_currentItem );
    }
    else
    {
        return nullptr;
    }
}

const SceneItem* Compositor::selectNext()
{
    if( !m_currentScene.m_sceneItems.empty() )
    {
        ++m_currentItem;
        if( m_currentItem == m_currentScene.m_sceneItems.end() )
        {
            m_currentItem = m_currentScene.m_sceneItems.begin();
        }

        if( !m_itemCursorEnabled )
        {
            m_cursorGlyph = 0x1e;
            m_cursorAttributes = 0x0e;
            m_terminal.createCursor( "Edit", m_currentItem->row(), m_currentItem->col(), m_cursorGlyph, m_cursorAttributes );
            m_itemCursorEnabled = true;
            m_cursorTimer->reset();
        }
        else
        {
            m_terminal.moveCursor( "Edit", m_currentItem->row(), m_currentItem->col() );
        }

        return &( *m_currentItem );
    }
    else
    {
        return nullptr;
    }    
}

const SceneItem* Compositor::selectPrevious()
{
    if( !m_currentScene.m_sceneItems.empty() )
    {
        if( m_currentItem == m_currentScene.m_sceneItems.begin() )
        {
            m_currentItem = m_currentScene.m_sceneItems.end();
        }
        --m_currentItem;

        if( !m_itemCursorEnabled )
        {
            m_cursorGlyph = 0x1e;
            m_cursorAttributes = 0x0e;
            m_terminal.createCursor( "Edit", m_currentItem->row(), m_currentItem->col(), m_cursorGlyph, m_cursorAttributes );
            m_itemCursorEnabled = true;
            m_cursorTimer->reset();
        }
        else
        {
            m_terminal.moveCursor( "Edit", m_currentItem->row(), m_currentItem->col() );
        }

        return &( *m_currentItem );
    }
    else
    {
        return nullptr;
    }    
}

const SceneItem* Compositor::selectDirection( enum Direction::_Direction direction )
{
    if( !m_currentScene.m_sceneItems.empty() )
    {
        double minEuclidean( 0.0 );
        Vector< SceneItem >::const_iterator closest( m_currentScene.m_sceneItems.end() );
        Vector< SceneItem >::const_iterator it( m_currentScene.m_sceneItems.begin() );
        for( ; it != m_currentScene.m_sceneItems.end(); ++it )
        {
            if( *m_currentItem != *it )
            {
                // Require matching objects to be in the right direction.
                // FIXME: Measure to object centres?
                if( ( ( direction == Direction::up ) && ( it->row() < m_currentItem->row() ) ) ||
                    ( ( direction == Direction::down ) && ( it->row() > m_currentItem->row() ) ) ||
                    ( ( direction == Direction::left ) && ( it->col() < m_currentItem->col() ) ) ||
                    ( ( direction == Direction::right ) && ( it->col() > m_currentItem->col() ) ) )
                {
                    int crossAxis( 0 );
                    if( ( direction == Direction::up ) || ( direction == Direction::down ) )
                    {
                        crossAxis = std::abs( m_currentItem->col() - it->col() );
                    }
                    else
                    {
                        crossAxis = std::abs( m_currentItem->row() - it->row() );
                    }

                    // Find closest on Euclidean distance.
                    double euclidean( ::sqrt( ::pow( ( it->col() - m_currentItem->col() ), 2 ) +
                                              ::pow( ( it->row() - m_currentItem->row() ), 2 ) ) );
                    euclidean += ( crossAxis * 3 ); // Penalty for cross-axis distance.
                    if( ( minEuclidean == 0.0 ) || ( euclidean < minEuclidean ) )
                    {
                        minEuclidean = euclidean;
                        closest = it;
                    }
                }
            }
        }

        if( closest != m_currentScene.m_sceneItems.end() )
        {
            m_currentItem = closest;

            if( !m_itemCursorEnabled )
            {
                m_cursorGlyph = 0x1e;
                m_cursorAttributes = 0x0e;
                m_terminal.createCursor( "Edit", m_currentItem->row(), m_currentItem->col(), m_cursorGlyph, m_cursorAttributes );
                m_itemCursorEnabled = true;
                m_cursorTimer->reset();
            }
            else
            {
                m_terminal.moveCursor( "Edit", m_currentItem->row(), m_currentItem->col() );
            }

            return &( *closest );
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
}

const SceneItem* Compositor::selectClosest( int row, int col )
{
    double minEuclidean( 0.0 );
    Vector< SceneItem >::const_iterator closest( m_currentScene.m_sceneItems.end() );
    Vector< SceneItem >::const_iterator it( m_currentScene.m_sceneItems.begin() );
    for( ; it != m_currentScene.m_sceneItems.end(); ++it )
    {
        if( *m_currentItem != *it )
        {
            // Find closest on Euclidean distance.
            double euclidean( ::sqrt( ::pow( ( it->col() - col ), 2 ) +
                                      ::pow( ( it->row() - row ), 2 ) ) );
            if( ( minEuclidean == 0.0 ) || ( euclidean < minEuclidean ) )
            {
                minEuclidean = euclidean;
                closest = it;
            }
        }
    }

    if( closest != m_currentScene.m_sceneItems.end() )
    {
        m_currentItem = closest;

        if( !m_itemCursorEnabled )
        {
            m_cursorGlyph = 0x1e;
            m_cursorAttributes = 0x0e;
            m_terminal.createCursor( "Edit", m_currentItem->row(), m_currentItem->col(), m_cursorGlyph, m_cursorAttributes );
            m_itemCursorEnabled = true;
            m_cursorTimer->reset();
        }
        else
        {
            m_terminal.moveCursor( "Edit", m_currentItem->row(), m_currentItem->col() );
        }

        return &( *closest );
    }
    else
    {
        return nullptr;
    }
}

const SceneItem* Compositor::selectBy( const String& snowflake )
{
    Vector< SceneItem >::const_iterator it( m_currentScene.m_sceneItems.begin() );
    for( ; it != m_currentScene.m_sceneItems.end(); ++it )
    {
        if( it->snowflake() == snowflake )
        {
            m_currentItem = it;

            if( !m_itemCursorEnabled )
            {
                m_cursorGlyph = 0x1e;
                m_cursorAttributes = 0x0e;
                m_terminal.createCursor( "Edit", m_currentItem->row(), m_currentItem->col(), m_cursorGlyph, m_cursorAttributes );
                m_itemCursorEnabled = true;
                m_cursorTimer->reset();
            }
            else
            {
                m_terminal.moveCursor( "Edit", m_currentItem->row(), m_currentItem->col() );
            }

            return &( *m_currentItem );
        }
    }

    selectNone();
    return nullptr;
}

void Compositor::selectNone()
{
    if( m_itemCursorEnabled )
    {
        m_terminal.deleteCursor( "Edit" );
        m_itemCursorEnabled = false;
    }
}

Set< const SceneItem* > Compositor::findNearbyItems( const SceneItem* sceneItem ) const
{
    Set< const SceneItem* > nearbyItems;

    // N.B. Callee will clip to the screen.
    collideBaseAndSprites( sceneItem->row() - 1,
                           sceneItem->col() - 1,
                           sceneItem->height() + 2,
                           sceneItem->width() + 2,
                           nearbyItems,
                           sceneItem );

    return nearbyItems;
}

Set< const ScenePresence* > Compositor::findNearbyUsers( const SceneItem* sceneItem ) const
{
    LiteStream stream;
    stream << "Nearby " << sceneItem->row() << "," << sceneItem->col()
           << " " << sceneItem->height() << "x" << sceneItem->width() << " : Users: ";

    Set< const ScenePresence* > scenePresences;

    Vector< ScenePresence >::const_iterator it( m_presences.begin() );
    for( ; it != m_presences.end(); ++it )
    {
        if( ( it->m_row >= ( sceneItem->row() - 1 ) ) &&
            ( it->m_row <= ( sceneItem->row() + sceneItem->height() ) ) &&
            ( it->m_col >= ( sceneItem->col() - 1 ) ) &&
            ( it->m_col <= ( sceneItem->col() + sceneItem->width() ) ) )
        {
            stream << it->m_user.m_snowflake << " " << it->m_row << "," << it->m_col << " ";
            scenePresences.insert( &( *it ) );
        }
    }

    LOG_DEBUG( stream.str() );

    return scenePresences;
}

Set< const SceneItem* > Compositor::findItems( const String& name ) const
{
    Set< const SceneItem* > sceneItems;

    Vector< SceneItem >::const_iterator it( m_currentScene.m_sceneItems.begin() );
    for( ; it != m_currentScene.m_sceneItems.end(); ++it )
    {
        if( it->assetName() == name )
        {
            sceneItems.insert( &( *it ) );
        }
    }

    return sceneItems;
}

const SceneItem* Compositor::findItemBySnowflake( const String& snowflake ) const
{
    Vector< SceneItem >::const_iterator it( m_currentScene.m_sceneItems.begin() );
    for( ; it != m_currentScene.m_sceneItems.end(); ++it )
    {
        if( it->snowflake() == snowflake )
        {
            return &( *it );
        }
    }

    return nullptr;
}

const SceneItem* Compositor::findItemByAction( const String& action ) const
{
    Vector< SceneItem >::const_iterator it( m_currentScene.m_sceneItems.begin() );
    for( ; it != m_currentScene.m_sceneItems.end(); ++it )
    {
        if( it->action().find( action ) != String::npos )
        {
            return &( *it );
        }
    }

    return nullptr;
}

const SceneItem* Compositor::itemAt( int row, int col, int height, int width ) const
{
    Set< const SceneItem* > itemsAt;

    collideBaseAndSprites( row,
                           col,
                           height,
                           width,
                           itemsAt,
                           nullptr );

    if( !itemsAt.empty() )
    {
        return *itemsAt.begin();
    }

    return nullptr;
}

int Compositor::getHeightAt( int row, int col )
{
    if( ( row < height() ) && ( col < width() ) ) // Terminal height and width
    {
        return( m_heightMap[ ( width() * row ) + col ] );
    }

    return 0;
}

void Compositor::setHeightAt( int row, int col, int newHeight )
{
    if( ( row < height() ) && ( col < width() ) ) // Terminal height and width
    {
        m_heightMap[ ( width() * row ) + col ] = newHeight;
    }
}

void Compositor::actionMessage()
{
    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _World );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _Action );
    tuple[_id] = m_user.m_snowflake;
    tuple[_name] = m_user.m_name;
    tuple[_row] = m_positionRow;
    tuple[_column] = m_positionCol;
    m_tupleRouter.route( tuple );
}

void Compositor::requestTeleport( Coordinates& teleportCoordinates, int row, int col )
{
    m_teleportCoordinates = teleportCoordinates;
    m_teleportRow = row;
    m_teleportCol = col;
    m_teleportRequested = true;
}

bool Compositor::teleportRequested()
{
    return m_teleportRequested;
}

void Compositor::getTeleportDest( Coordinates& teleportCoordinates, int& row, int& col )
{
    teleportCoordinates = m_teleportCoordinates;
    row = m_teleportRow;
    col = m_teleportCol;
}

void Compositor::clearTeleportRequested()
{
    m_teleportRequested = false;
}

bool Compositor::hasSceneItemAttribute( const String& snowflake,
                                        const String& name )
{
    if( m_currentSceneLoader )
    {
        return m_currentSceneLoader->hasSceneItemAttribute( snowflake, name );
    }

    return false;
}

bool Compositor::createSceneItemAttribute( const String& snowflake,
                                           const String& name )
{
    if( m_currentSceneLoader )
    {
        return m_currentSceneLoader->createSceneItemAttribute( snowflake, name );
    }

    return false;
}

bool Compositor::loadSceneItemAttribute( const String& snowflake,
                                         const String& name,
                                         Value& value )
{
    if( m_currentSceneLoader )
    {
        return m_currentSceneLoader->loadSceneItemAttribute( snowflake, name, value );
    }

    return false;
}

bool Compositor::saveSceneItemAttribute( const String& snowflake,
                                         const String& name,
                                         const Value& value )
{
    if( m_currentSceneLoader )
    {
        return m_currentSceneLoader->saveSceneItemAttribute( snowflake, name, value );
    }

    return false;
}

bool Compositor::deleteSceneItemAttributes( const String& snowflake )
{
    if( m_currentSceneLoader )
    {
        return m_currentSceneLoader->deleteSceneItemAttributes( snowflake );
    }

    return false;
}

void Compositor::invalidateAllCached()
{
    LOG_DEBUG( "Compositor: Invalidating all cached assets" );
    AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, String() ) );
    AssetLoader* programAssetLoader( m_programAssetLoaderFactory.makeLoader( m_coordinates, String() ) );
    assetLoader->invalidateCached( true ); // true = all.
    programAssetLoader->invalidateCached( true ); // true = all.
    delete( assetLoader );
    delete( programAssetLoader );
}

void Compositor::invalidateCached( const Vector< struct SceneLoader::InvalidatedAsset >& invalidatedAssets,
                                   bool& didRender )
{
    bool needRender( false );

    Vector< struct SceneLoader::InvalidatedAsset >::const_iterator invalidatedIt( invalidatedAssets.begin() );
    for( ; invalidatedIt != invalidatedAssets.end(); ++invalidatedIt )
    {
        if( invalidatedIt->m_originatorID == m_tupleRouter.myID() )
        {
            // We will have already handled our own requests.
            continue;
        }

        LOG_DEBUG( "Compositor: Handling cached asset invalidation for " + invalidatedIt->m_name + " type " + invalidatedIt->m_type );
        if( invalidatedIt->m_type == _asset )
        {
            AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, invalidatedIt->m_name ) );
            assetLoader->invalidateCached();

            Vector< SceneItem >::const_iterator sceneIt( m_currentScene.m_sceneItems.begin() );
            for( ; sceneIt != m_currentScene.m_sceneItems.end(); ++sceneIt )
            {
                if( sceneIt->assetName() == invalidatedIt->m_name )
                {
                    LOG_DEBUG( "Compositor: Found in current scene. Need render." );
                    needRender = true;
                }
            }

            delete( assetLoader );
        }
        else if( invalidatedIt->m_type == _program )
        {
            AssetLoader* programAssetLoader( m_programAssetLoaderFactory.makeLoader( m_coordinates, invalidatedIt->m_name ) );
            programAssetLoader->invalidateCached();

            if( m_programManager.isLoaded( invalidatedIt->m_name ) )
            {
                LOG_DEBUG( "Compositor: Found in currently loaded programs. Need render." );
                needRender = true;
            }

            delete( programAssetLoader );
        }
    }

    if( needRender )
    {
        render();
        didRender = true;
    }
}

void Compositor::notifyInvalidated( const String& name,
                                    const String& type,
                                    bool& didRender )
{
    struct SceneLoader::InvalidatedAsset invalidatedAsset;
    invalidatedAsset.m_name = name;
    invalidatedAsset.m_type = type;
    invalidatedAsset.m_originatorID = m_tupleRouter.myID();

    if( m_currentSceneLoader )
    {
        LOG_DEBUG( "Compositor: Requesting invalidation for cached asset " + name + " with type " + type );
        m_currentSceneLoader->invalidateCachedAsset( invalidatedAsset );
    }

    Vector< struct SceneLoader::InvalidatedAsset > invalidatedAssets;
    invalidatedAssets.push_back( invalidatedAsset );
    invalidateCached( invalidatedAssets, didRender );
}

void Compositor::lockScene( bool lock )
{
    m_sceneLocked = lock;
}

void Compositor::setCursorsVisible( bool visible )
{
    m_terminal.setCursorsVisible( visible );
}

void Compositor::clearScreen()
{
    m_terminal.clearScreen();
}

void Compositor::run()
{
    // If we're idle, switch to idle glyph.
    if( ( m_glyphOffset != 0 ) && ( m_idleTimer->ms() >= idleTimeout ) && m_currentPresenceLoader )
    {
        m_glyphOffset = 0;

        if( !m_positionHidden )
        {
            m_terminal.moveCursor( m_user.m_snowflake, m_positionRow, m_positionCol, m_user.m_glyph );

            ScenePresence scenePresence;
            scenePresence.m_user = m_user;
            scenePresence.m_coordinates = m_coordinates;
            scenePresence.m_row = m_positionRow;
            scenePresence.m_col = m_positionCol;

            PresenceRequest presenceRequest( PresenceRequest::move, scenePresence, m_coordinates );

            Vector< PresenceRequest > presenceRequests;
            presenceRequests.push_back( presenceRequest );
            m_currentPresenceLoader->request( presenceRequests );

            updatePresences( presenceRequests ); // Perform local update immediately.

            m_pendingMyPresenceMoveRequests.clear(); // Remove any pending redundant updates.
        }
    }

    // If the item cursor is enabled, animate it.
    if( m_itemCursorEnabled && ( m_cursorTimer->ms() >= cursorPeriod ) )
    {
        m_cursorGlyph = ( m_cursorGlyph == 0x1e ) ? 0x1f : 0x1e;
        if( m_cursorGlyph == 0x1f )
        {
            m_cursorAttributes = ( m_cursorAttributes == 0x0e ) ? 0x00 : 0x0e;
        }
        m_terminal.moveCursor( "Edit", m_currentItem->row(), m_currentItem->col(), m_cursorGlyph, m_cursorAttributes );
        
        m_cursorTimer->reset();
    }
    
    if( !m_sceneLocked )
    {
        // Periodically send the latest move request to the server
        // (server update throttling)
        if( m_moveRequestTimer->ms() >= moveRequestPeriod )
        {
            if( m_currentPresenceLoader && !m_pendingRemotePresenceRequests.empty() )
            {
                m_currentPresenceLoader->request( m_pendingRemotePresenceRequests );
                m_pendingRemotePresenceRequests.clear();
            }

            if( m_currentPresenceLoader && !m_pendingMyPresenceMoveRequests.empty() )
            {
                m_currentPresenceLoader->request( m_pendingMyPresenceMoveRequests );
                m_pendingMyPresenceMoveRequests.clear();
            }

            if( m_currentSceneLoader && !m_pendingRemoteSceneRequests.empty() )
            {
                m_currentSceneLoader->request( m_pendingRemoteSceneRequests );
                m_pendingRemoteSceneRequests.clear();
            }

            m_moveRequestTimer->reset();
        }

        // Handle queued locally generated scene and presence update
        // requests immediately.
        if( !m_pendingLocalSceneRequests.empty() )
        {
            updateScene( m_pendingLocalSceneRequests );
            m_pendingLocalSceneRequests.clear();
        }

        if( !m_pendingLocalPresenceRequests.empty() )
        {
            updatePresences( m_pendingLocalPresenceRequests );
            m_pendingLocalPresenceRequests.clear();
        }

        // Handle scene and presence update requests received from the server.
        // We'll get our own requests back, but we just drop them as we'll have
        // already processed them.
        if( m_loadersUpdateTimer->ms() >= loadersUpdatePeriod )
        {
            // Don't allow the compsitor to poll for scene updates
            // while editing is in progress. In the case of two clients
            // editing the same thing, let the server deconflict it
            // (the loser's edit will fail), then roll out all scene
            // updates when out of editing mode (editor unlocks scene).
            if( m_currentSceneLoader )
            {
                if( !m_currentSceneLoader->overflowed() )
                {
                    Vector< SceneRequest > requests( m_currentSceneLoader->getUpdates() );
                    if( !requests.empty() )
                    {
                        updateScene( requests );
                    }
                }
                else
                {
                    // Reload whole scene.
                    LOG_DEBUG( "Compositor: Reloading scene due to request overflow" );
                    m_currentSceneLoader->load( m_currentScene );
                }

                Vector< struct SceneLoader::InvalidatedAsset > invalidatedAssets( m_currentSceneLoader->getInvalidatedAssets() );
                if( !invalidatedAssets.empty() )
                {
                    bool didRender( false );
                    invalidateCached( invalidatedAssets, didRender );
                }
            }

            if( m_currentPresenceLoader )
            {
                if( !m_currentPresenceLoader->overflowed() )
                {
                    Vector< PresenceRequest > requests( m_currentPresenceLoader->getUpdates() );
                    if( !requests.empty() )
                    {
                        updatePresences( requests );
                    }
                }
                else
                {
                    // Reload all presences.
                    LOG_DEBUG( "Compositor: Reloading presences due to request overflow" );
                    m_currentPresenceLoader->load( m_presences );
                }
            }

            m_loadersUpdateTimer->reset();
        }
    }

    m_terminal.run(); // Animate characters.
}

void Compositor::tileBackground()
{
    AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, _ground ) );
    if( !assetLoader->open() )
    {
        delete( assetLoader );
        assetLoader = m_unknownAssetLoaderFactory.makeLoader( Coordinates(), _unknown );
        assetLoader->open();
    }

    Assets::ANSIFile ansiFile( *assetLoader );

    int tileWidth = ansiFile.width();
    int tileHeight = ansiFile.height();
    for( int row = 0; row < height(); row += tileHeight )
    {
        for( int col = 0; col < width(); col += tileWidth )
        {
            m_terminal.consumeNext( row, col );
            m_terminal.consumeAsset( ansiFile,
                                     0,
                                     ansiFile.dataSize(),
                                     tileWidth,
                                     col,
                                     Terminal::noMaxRow,
                                     Terminal::scrollLock,
                                     Terminal::ANSI );
        }
    }

    delete( assetLoader );
}

void Compositor::_drawTextItem( const SceneItem& sceneItem, const String& text, bool clearFirst )
{
    if( clearFirst )
    {
        m_terminal.repaint( sceneItem.row(),
                            sceneItem.col(),
                            sceneItem.height(),
                            sceneItem.width() );
    }

    Tokeniser lineTokeniser( text );
    String line( lineTokeniser.token() );
    int currentRow( sceneItem.row() + textBorder );
    int rowsRemain( sceneItem.height() - ( textBorder * 2 ) );
    while( rowsRemain > 0 )
    {
        int rowsPrinted( 0 );
        if( line[line.size()-1] == '\r' ) line.pop_back();
        if( !line.empty() )
        {
            rowsPrinted = m_terminal.printFormatted( line,
                                                     currentRow,
                                                     sceneItem.col() + textBorder,
                                                     rowsRemain,
                                                     sceneItem.width() - ( textBorder * 2 ),
                                                     Terminal::hCentre,
                                                     Terminal::noVCentre,
                                                     textAttributes,
                                                     Terminal::preserveBackground );
        }
        else
        {
            rowsPrinted = 1;
        }

        if( lineTokeniser.atEnd() ) break;
        line = lineTokeniser.token();
        currentRow += rowsPrinted;
        rowsRemain -= rowsPrinted;
    }
}

void Compositor::getAssetDimensions( const String& assetName, int& height, int& width )
{
    AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, assetName ) );
    if( !assetLoader->open() )
    {
        delete( assetLoader );
        assetLoader = m_unknownAssetLoaderFactory.makeLoader( Coordinates(), "unknown" );
        assetLoader->open();
    }

    Assets::ANSIFile ansiFile( *assetLoader );

    bool blit( false );
    bool sprite( false );
    bool animate( false );
    int frames( 0 );
    String templateName;
    if( ansiFile.hasSAUCE() )
    {
        ansiFile.getAssetFlags( blit, sprite, animate, frames, templateName );
    }

    height = animate ? ( ansiFile.height() / frames ) : ansiFile.height();
    width = ansiFile.width();

    delete( assetLoader );
}

void Compositor::loadTemplateProgram( const String& itemSnowflake, const String& templateName )
{
    if( !templateName.empty() )
    {
        if( m_programManager.load( m_programAssetLoaderFactory, m_coordinates, itemSnowflake, templateName ) )
        {
            LOG_DEBUG( "Loaded template program " + templateName );
            m_programs.push_back( itemSnowflake );
        }
    }
}

void Compositor::loadLinkedProgram( const String& itemSnowflake )
{
    if( m_programManager.load( m_programAssetLoaderFactory, m_coordinates, itemSnowflake ) )
    {
        LOG_DEBUG( "Loaded program " + itemSnowflake );
        m_programs.push_back( itemSnowflake );
    }
}

void Compositor::tryWalk( enum Direction::_Direction direction, int newRow, int newCol, int glyphOffset1, int glyphOffset2 )
{
    bool hiding( false );
    bool unhiding( false );

    int baseHeight( m_heightMap[ ( width() * newRow ) + newCol ] );

    Vector< String > spriteNames;
    m_terminal.collideSprite( newRow, newCol, 1, 1, spriteNames );
    int maxSpriteHeight( 0 );
    Vector< String >::const_iterator spriteNameIt( spriteNames.begin() );
    for( ; spriteNameIt != spriteNames.end(); ++spriteNameIt )
    {
        const String& spriteName( *spriteNameIt );

        String assetName;
        int spriteRow( 0 );
        int spriteCol( 0 );
        int spriteHeight( 0 );
        int spriteWidth( 0 );
        if( m_terminal.spriteData( spriteName, assetName, spriteRow, spriteCol, spriteHeight, spriteWidth ) )
        {
            char* spriteMap;
            if( m_terminal.spriteMap( *spriteNameIt, spriteMap ) )
            {
                int spriteRowOffset( newRow - spriteRow );
                int spriteColOffset( newCol - spriteCol );
                int thisSpriteHeight( *( spriteMap + ( spriteRowOffset * spriteWidth ) + spriteColOffset ) );
                if( thisSpriteHeight > maxSpriteHeight )
                {
                    maxSpriteHeight = thisSpriteHeight;
                }
            }
        }
    }

    int maxHeight( ( maxSpriteHeight > baseHeight ) ? maxSpriteHeight : baseHeight );

    if( maxHeight == background )
    {
        m_positionRow = newRow;
        m_positionCol = newCol;

        if( m_positionHidden )
        {
            m_positionHidden = false;
            unhiding = true;
        }

        if( m_walkCycle == 0 )
        {
            m_glyphOffset = glyphOffset1;
            m_walkCycle = 1;
        }
        else
        {
            m_glyphOffset = glyphOffset2;
            m_walkCycle = 0;
        }

        m_idleTimer->reset();
    }
    else if( maxHeight == foreground )
    {
        m_positionRow = newRow;
        m_positionCol = newCol;

        if( !m_positionHidden )
        {
            m_positionHidden = true;
            hiding = true;
        }
    }
    // else if ground (collision), don't move at all.

    if( hiding || unhiding || !m_positionHidden )
    {
        ScenePresence scenePresence;
        scenePresence.m_user = m_user;
        scenePresence.m_user.m_glyph += m_glyphOffset;
        scenePresence.m_coordinates = m_coordinates;
        scenePresence.m_row = m_positionRow;
        scenePresence.m_col = m_positionCol;

        enum PresenceRequest::PresenceOperation presenceOperation( PresenceRequest::move );
        if( hiding ) presenceOperation = PresenceRequest::depart;
        if( unhiding ) presenceOperation = PresenceRequest::arrive;

        bool push( maxHeight == ground );

        PresenceRequest request( presenceOperation, scenePresence, m_coordinates, direction, push, true ); // true = keyboard
        Vector< PresenceRequest > presenceRequests;
        presenceRequests.push_back( request );

        if( !hiding && !unhiding )
        {
            // Delayed remote send.
            m_pendingMyPresenceMoveRequests.clear();
            m_pendingMyPresenceMoveRequests.push_back( request );

            // Immediate local update (handled in run()).
            if( m_pendingLocalPresenceRequests.size() == maxQueuedRequests )
            {
                m_pendingLocalPresenceRequests.pop_back(); // Make space.
            }
            m_pendingLocalPresenceRequests.push_back( request );
        }
        else
        {
            // Handle immediately, remote.
            if( m_currentPresenceLoader )
            {
                m_currentPresenceLoader->request( presenceRequests );
            }

            // Handle immediately, local.
            if( m_pendingLocalPresenceRequests.size() == maxQueuedRequests )
            {
                m_pendingLocalPresenceRequests.pop_back(); // Make space.
            }
            m_pendingLocalPresenceRequests.push_back( request );

            // Kill any pending move requests.
            m_pendingMyPresenceMoveRequests.clear();
        }
    }
}

void Compositor::updateScene( Vector< SceneRequest >& requests )
{
    Vector< Tuple > tuples;

    bool needRender( false );

    std::sort( requests.begin(), requests.end() );

    String currentItemSnowflake;
    if( m_currentItem != m_currentScene.m_sceneItems.end() )
    {
        currentItemSnowflake = m_currentItem->snowflake();
    }

    Vector< SceneRequest >::const_iterator requestIt;
    for( requestIt = requests.begin(); requestIt != requests.end(); ++requestIt )
    {
        if( ( requestIt->m_coordinates != m_coordinates ) &&
            ( requestIt->m_newCoordinates != m_coordinates ) )
        {
            // Shouldn't happen if server is filtering properly...
            LOG_DEBUG( "Received bogus scene update!" );
            continue;
        }
        
        if( requestIt->m_originatorID == m_tupleRouter.myID() )
        {
            // We will have already applied our own requests.
            // FIXME: Back out our local changes here if the request was denied
            // by the server (this can be as simple as just forcing a complete
            // re-render, to reload the entire scene from the server).
            continue;
        }

        LOG_DEBUG( "Received scene update requests" );
        switch( requestIt->m_sceneOperation )
        {
        case SceneRequest::create:
            // FIXME: If we're creating or deleting a sprite here, we don't
            // need to re-render the entire scene. On that note, we might
            // consider making everthing a "sprite" and only flattening
            // the background tiles.
            LOG_DEBUG( "Received scene create request" );
            m_currentScene.m_sceneItems.push_back( requestIt->m_sceneItem );
            needRender = true;
            break;
        case SceneRequest::update:
            {
            Vector< SceneItem >::iterator sceneItemsIt;
            for( sceneItemsIt = m_currentScene.m_sceneItems.begin(); sceneItemsIt != m_currentScene.m_sceneItems.end(); ++sceneItemsIt )
            {
                if( *sceneItemsIt == requestIt->m_sceneItem )
                {
                    SceneItem& currentSceneItem( *sceneItemsIt );
                    const SceneItem& newSceneItem( requestIt->m_sceneItem );

                    if( isSprite( newSceneItem.snowflake() ) &&
                        ( currentSceneItem.assetName() == newSceneItem.assetName() ) &&
                        ( currentSceneItem.action() == newSceneItem.action() ) &&
                        ( currentSceneItem.flags() == newSceneItem.flags() ) )
                    {
                        moveSprite( newSceneItem.snowflake(), newSceneItem.row(), newSceneItem.col() );
                    }
                    else
                    {
                        needRender = true;
                    }

                    currentSceneItem = newSceneItem;

                    // FIXME: This was previously sent by WorldActor in response
                    // to SceneResponse tuples, but needed to be moved here
                    // as we can't send this to user code until our scene is
                    // actually updated. Compositor has become a God class that
                    // needs to be re-factored so something else is responsible
                    // for the scene state ground-truth.
                    Tuple sendTuple;
                    TupleRouter::setSourceActor( sendTuple, _World );
                    TupleRouter::setSourceID( sendTuple, m_tupleRouter.myID() );
                    TupleRouter::setDestinationActor( sendTuple, newSceneItem.snowflake() );
                    TupleRouter::setTupleType( sendTuple, _ThingChanged );
                    sendTuple[_name] = newSceneItem.assetName();
                    sendTuple[_row] = newSceneItem.row();
                    sendTuple[_column] = newSceneItem.col();
                    sendTuple[_id] = newSceneItem.snowflake();
                    sendTuple[_computerid] = !requestIt->m_originatorID.empty() ? requestIt->m_originatorID : m_tupleRouter.myID();
                    sendTuple[_keyboard] = ( requestIt->m_keyboard ) ? 1 : 0;
                    requestIt->m_direction.toValue( sendTuple[_direction] );

                    sendTuple[_edge] = _none;
                    if( newSceneItem.row() <= 0 ) sendTuple[_edge] = _top;
                    if( ( newSceneItem.row() + newSceneItem.height() ) >= height() ) sendTuple[_edge] = _bottom;
                    if( newSceneItem.col() <= 0 ) sendTuple[_edge] = _left;
                    if( ( newSceneItem.col() + newSceneItem.width() ) >= width() ) sendTuple[_edge] = _right;

                    tuples.push_back( sendTuple );

                    collideItem( *requestIt );
                }
            }
            }
            break;
        case SceneRequest::remove:
            {
            Vector< SceneItem >::iterator sceneItemsIt;
            for( sceneItemsIt = m_currentScene.m_sceneItems.begin(); sceneItemsIt != m_currentScene.m_sceneItems.end(); ++sceneItemsIt )
            {
                if( *sceneItemsIt == requestIt->m_sceneItem )
                {
                    m_currentScene.m_sceneItems.erase( sceneItemsIt );
                    needRender = true;
                    break;
                }
            }
            }
            break;
        case SceneRequest::transport:
            if( requestIt->m_coordinates == m_coordinates )
            {
                // Transport source - remove item.
                LOG_DEBUG( "Received scene transport request - source. Removing item." );
                {
                Vector< SceneItem >::iterator sceneItemsIt;
                for( sceneItemsIt = m_currentScene.m_sceneItems.begin(); sceneItemsIt != m_currentScene.m_sceneItems.end(); ++sceneItemsIt )
                {
                    if( *sceneItemsIt == requestIt->m_sceneItem )
                    {
                        m_currentScene.m_sceneItems.erase( sceneItemsIt );
                        needRender = true;
                        break;
                    }
                }
                }
            }
            else if( requestIt->m_newCoordinates == m_coordinates )
            {
                // Transport destination - create item.
                LOG_DEBUG( "Received scene transport request - destination. Creating item." );
                m_currentScene.m_sceneItems.push_back( requestIt->m_sceneItem );
                needRender = true;
            }
            break;
        case SceneRequest::raise:
            {
            SceneItem movedItem;
            bool found( false );
            Vector< SceneItem >::iterator sceneItemsIt;
            for( sceneItemsIt = m_currentScene.m_sceneItems.begin(); sceneItemsIt != m_currentScene.m_sceneItems.end(); ++sceneItemsIt )
            {
                if( *sceneItemsIt == requestIt->m_sceneItem )
                {
                    movedItem = *sceneItemsIt;
                    m_currentScene.m_sceneItems.erase( sceneItemsIt );
                    found = true;
                    break;
                }
            }

            if( found )
            {
                m_currentScene.m_sceneItems.insert( m_currentScene.m_sceneItems.end(), movedItem );
                needRender = true;
            }
            }
            break;
        case SceneRequest::lower:
            {
            SceneItem movedItem;
            bool found( false );
            Vector< SceneItem >::iterator sceneItemsIt;
            for( sceneItemsIt = m_currentScene.m_sceneItems.begin(); sceneItemsIt != m_currentScene.m_sceneItems.end(); ++sceneItemsIt )
            {
                if( *sceneItemsIt == requestIt->m_sceneItem )
                {
                    movedItem = *sceneItemsIt;
                    m_currentScene.m_sceneItems.erase( sceneItemsIt );
                    found = true;
                    break;
                }
            }

            if( found )
            {
                m_currentScene.m_sceneItems.insert( m_currentScene.m_sceneItems.begin(), movedItem );
                needRender = true;
            }
            }
            break;
        default:
            LiteStream debugStream;
            debugStream << "Received scene request with opcode " << requestIt->m_sceneOperation;
            LOG_DEBUG( debugStream.str() );
        }
    }

    if( !currentItemSnowflake.empty() )
    {
        // Try to restore last selected item.
        Vector< SceneItem >::const_iterator sceneItemsIt( m_currentScene.m_sceneItems.begin() );
        for( ; sceneItemsIt != m_currentScene.m_sceneItems.end(); ++sceneItemsIt )
        {
            if( sceneItemsIt->snowflake() == currentItemSnowflake )
            {
                m_currentItem = sceneItemsIt;
                break;
            }
        }

        if( sceneItemsIt == m_currentScene.m_sceneItems.end() )
        {
            // Can't restore last selected item. Possibly deleted now?
            m_currentItem = m_currentScene.m_sceneItems.end();
            if( !m_currentScene.m_sceneItems.empty() )
            {
                m_currentItem--; // Select last item.
            }
        }
    }

    if( needRender )
    {
        LOG_DEBUG( "All scene requests handled. Rendering." );
        render();
    }

    // render() will reload all programs - delay sending tuples until after,
    // so any runtime errors generated while handling those tuples will
    // be generated after reload and therefore retained.
    Vector< Tuple >::iterator tuplesIt( tuples.begin() );
    for( ; tuplesIt != tuples.end(); ++tuplesIt )
    {
        m_tupleRouter.route( *tuplesIt );
    }
}

void Compositor::updatePresences( Vector< PresenceRequest >& requests )
{
    Vector< PresenceRequest >::const_iterator requestIt( requests.begin() );
    for( ; requestIt != requests.end(); ++requestIt )
    {
        const PresenceRequest& request( *requestIt );
        if( request.m_coordinates != m_coordinates )
        {
            // Shouldn't happen if server is filtering properly...
            LOG_DEBUG( "Received bogus presence update!" );
            continue;
        }

        if( request.m_originatorID == m_tupleRouter.myID() )
        {
            // We will have already applied our own requests.
            // FIXME: Back out our local changes here if the request was denied
            // by the server (this can be as simple as just forcing a complete
            // re-render, to reload all scene presences from the server).
            continue;
        }

        switch( request.m_presenceOperation )
        {
        case PresenceRequest::arrive:
            {
            // Check for old presence and cursor and erase.
            Vector< ScenePresence >::iterator scenePresencesIt( m_presences.begin() );
            for( ; scenePresencesIt != m_presences.end(); ++scenePresencesIt )
            {
                const ScenePresence& scenePresence( *scenePresencesIt );
                if( scenePresence.m_user.m_name == request.m_scenePresence.m_user.m_name )
                {
                    m_presences.erase( scenePresencesIt );
                    m_terminal.deleteCursor( request.m_scenePresence.m_user.m_snowflake );
                    break;
                }
            }

            // Add new presence and cursor.
            m_presences.push_back( request.m_scenePresence );
            m_terminal.createCursor( request.m_scenePresence.m_user.m_snowflake,
                                    request.m_scenePresence.m_row,
                                    request.m_scenePresence.m_col,
                                    request.m_scenePresence.m_user.m_glyph,
                                    request.m_scenePresence.m_user.m_attributes,
                                    Terminal::avatarCharset );
            // FIXME: Send a local PersonMoved like below, or PersonArrived?
            collidePerson( request );
            }
            break;
        case PresenceRequest::depart:
            {
            // Erase presence and cursor.
            Vector< ScenePresence >::iterator scenePresencesIt( m_presences.begin() );
            for( ; scenePresencesIt != m_presences.end(); ++scenePresencesIt )
            {
                const ScenePresence& scenePresence( *scenePresencesIt );
                if( scenePresence.m_user.m_name == request.m_scenePresence.m_user.m_name )
                {
                    m_presences.erase( scenePresencesIt );
                    m_terminal.deleteCursor( request.m_scenePresence.m_user.m_snowflake );
                    break;
                }
            }
            }
            break;
        case PresenceRequest::move:
            {
            // Check for existing presence and update.
            Vector< ScenePresence >::iterator scenePresencesIt( m_presences.begin() );
            for( ; scenePresencesIt != m_presences.end(); ++scenePresencesIt )
            {
                ScenePresence& scenePresence( *scenePresencesIt );
                if( scenePresence.m_user.m_name == request.m_scenePresence.m_user.m_name )
                {
                    // FIXME: Check height map to show/hide cursor.
                    scenePresence.m_user = request.m_scenePresence.m_user;
                    scenePresence.m_row = request.m_scenePresence.m_row;
                    scenePresence.m_col = request.m_scenePresence.m_col;

                    // Move existing cursor
                    m_terminal.moveCursor( request.m_scenePresence.m_user.m_snowflake,
                                           request.m_scenePresence.m_row,
                                           request.m_scenePresence.m_col,
                                           request.m_scenePresence.m_user.m_glyph,
                                           request.m_scenePresence.m_user.m_attributes );
                    
                    if( scenePresence.m_user == m_user )
                    {
                        // If we moved, update local variables for our
                        // own position.
                        m_positionRow = scenePresence.m_row;
                        m_positionCol = scenePresence.m_col;
                    }

                    // FIXME: As per ThingChanged (see above), this has to be
                    // sent here to ensure that scene presences in Compositor
                    // are appropriately updated first. Should be re-factored
                    // so something else manages presence ground-truth.
                    Tuple sendTuple;
                    TupleRouter::setSourceActor( sendTuple, _World );
                    TupleRouter::setSourceID( sendTuple, m_tupleRouter.myID() );
                    TupleRouter::setTupleType( sendTuple, _PersonMoved );
                    sendTuple[_id] = request.m_scenePresence.m_user.m_snowflake;
                    sendTuple[_name] = request.m_scenePresence.m_user.m_name;
                    sendTuple[_row] = request.m_scenePresence.m_row;
                    sendTuple[_column] = request.m_scenePresence.m_col;
                    sendTuple[_computerid] = !request.m_originatorID.empty() ? request.m_originatorID : m_tupleRouter.myID();
                    sendTuple[_push] = ( request.m_push ) ? 1 : 0;
                    sendTuple[_keyboard] = ( request.m_keyboard ) ? 1 : 0;
                    request.m_direction.toValue( sendTuple[_direction] );

                    m_tupleRouter.route( sendTuple );

                    collidePerson( request );

                    break;
                }
            }

            if( scenePresencesIt == m_presences.end() )
            {
                // We didn't get an arrive or see them on scene load...
                // FIXME: Check height map to show/hide cursor.
                m_presences.push_back( request.m_scenePresence );

                m_terminal.createCursor( request.m_scenePresence.m_user.m_snowflake,
                                         request.m_scenePresence.m_row,
                                         request.m_scenePresence.m_col,
                                         request.m_scenePresence.m_user.m_glyph,
                                         request.m_scenePresence.m_user.m_attributes,
                                         Terminal::avatarCharset );
            }
            }
            break;
        default:
            break;
        }
    }
}

void Compositor::collideBaseAndSprites( int row,
                                        int col,
                                        int height,
                                        int width,
                                        Set< const SceneItem* >& sceneItemsCollided,
                                        const SceneItem* colliderSceneItem ) const
{
    // Find base items.
    for( int x = 0; x < width; ++x )
    {
        for( int y = 0; y < height; ++y )
        {
            int offset( ( ( row + y ) * m_terminal.width() ) + ( col + x ) );
            int itemIdx( m_collisionMap[offset] );
            if( itemIdx != -1 )
            {
                const SceneItem& collidingSceneItem( m_currentScene.m_sceneItems[itemIdx] );
                if( !colliderSceneItem || ( *colliderSceneItem != collidingSceneItem ) )
                {
                    sceneItemsCollided.insert( &collidingSceneItem );
                }
            }
        }
    }

    // Find sprite intersections.
    Vector< String > snowflakes;
    m_terminal.collideSprite( row,
                              col,
                              height,
                              width,
                              snowflakes );
    Vector< String >::const_iterator snowflakesIt( snowflakes.begin() );
    for( ; snowflakesIt != snowflakes.end(); ++snowflakesIt )
    {
        Vector< SceneItem >::const_iterator sceneItemsIt( m_currentScene.m_sceneItems.begin() );
        for( ; sceneItemsIt != m_currentScene.m_sceneItems.end(); ++sceneItemsIt )
        {
            const SceneItem& collidingSceneItem( *sceneItemsIt );
            if( sceneItemsIt->snowflake() == *snowflakesIt )
            {
                if( !colliderSceneItem || ( *colliderSceneItem != collidingSceneItem ) )
                {
                    sceneItemsCollided.insert( &collidingSceneItem );
                }
                break;
            }
        }
    }
}

void Compositor::collideBaseAndSprites( int row,
                                        int col,
                                        Set< const SceneItem* >& sceneItemsCollided,
                                        const SceneItem* colliderSceneItem ) const
{
    collideBaseAndSprites( row, col, 1, 1, sceneItemsCollided, colliderSceneItem );
}

void Compositor::collideItem( const SceneRequest& request )
{
    const SceneItem& sceneItem( request.m_sceneItem );

    Set< const SceneItem* > collidedSceneItems;

    collideBaseAndSprites( sceneItem.row(),
                           sceneItem.col(),
                           sceneItem.height(),
                           sceneItem.width(),
                           collidedSceneItems,
                           &sceneItem );

    Set< const SceneItem* >::const_iterator it( collidedSceneItems.begin() );
    for( ; it != collidedSceneItems.end(); ++it )
    {
        const SceneItem* collidedItem( *it );

        // TODO: Perform local action?

        // Send crash tuple to crashed item.
        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _World );
        TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
        TupleRouter::setDestinationActor( tuple, collidedItem->snowflake() );
        TupleRouter::setDestinationID( tuple, m_tupleRouter.myID() );
        TupleRouter::setTupleType( tuple, _Crash );
        tuple[_id] = collidedItem->snowflake();
        tuple[_name] = collidedItem->assetName();
        tuple[_row] = collidedItem->row();
        tuple[_column] = collidedItem->col();
        tuple[_computerid] = !request.m_originatorID.empty() ? request.m_originatorID : m_tupleRouter.myID();
        tuple[_keyboard] = ( request.m_keyboard ) ? 1 : 0;
        request.m_direction.toValue( tuple[_direction] );

        if( ( ( request.m_direction.m_direction == Direction::right ) && ( collidedItem->col() == collidedItem->col() ) ) ||
            ( ( request.m_direction.m_direction == Direction::left ) && ( collidedItem->col() == ( collidedItem->col() + collidedItem->width() - 1 ) ) ) ||
            ( ( request.m_direction.m_direction == Direction::down ) && ( collidedItem->row() == collidedItem->row() ) ) ||
            ( ( request.m_direction.m_direction == Direction::up ) && ( collidedItem->row() == ( collidedItem->row() + collidedItem->height() - 1 ) ) ) )
        {
            tuple[_edge] = 1;
        }
        else
        {
            tuple[_edge] = 0;
        }
        
        m_tupleRouter.route( tuple );
    }
}

void Compositor::collidePerson( const PresenceRequest& request )
{
    const ScenePresence& scenePresence( request.m_scenePresence );

    int collideRow( scenePresence.m_row );
    int collideCol( scenePresence.m_col );

    if( request.m_push )
    {
        if( request.m_direction.m_direction == Direction::up ) --collideRow;
        if( request.m_direction.m_direction == Direction::down ) ++collideRow;
        if( request.m_direction.m_direction == Direction::right ) ++collideCol;
        if( request.m_direction.m_direction == Direction::left ) --collideCol;
    }

    Set< const SceneItem* > collidedSceneItems;

    collideBaseAndSprites( collideRow,
                           collideCol,
                           collidedSceneItems );

    Set< const SceneItem* >::const_iterator it( collidedSceneItems.begin() );
    for( ; it != collidedSceneItems.end(); ++it )
    {
        const SceneItem* collidedItem( *it );

        // Perform local bump action, if any.
        tryPerformAction( *collidedItem, bumpAction );

        // Send bump tuple to bumped item.
        Tuple tuple;
        TupleRouter::setSourceActor( tuple, _World );
        TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
        TupleRouter::setDestinationActor( tuple, collidedItem->snowflake() );
        TupleRouter::setDestinationID( tuple, m_tupleRouter.myID() );
        TupleRouter::setTupleType( tuple, _Bump );
        tuple[_id] = scenePresence.m_user.m_snowflake;
        tuple[_name] = scenePresence.m_user.m_name;
        tuple[_row] = scenePresence.m_row;
        tuple[_column] = scenePresence.m_col;
        tuple[_computerid] = !request.m_originatorID.empty() ? request.m_originatorID : m_tupleRouter.myID();
        tuple[_keyboard] = ( request.m_keyboard ) ? 1 : 0;
        request.m_direction.toValue( tuple[_direction] );

        if( ( ( request.m_direction.m_direction == Direction::right ) && ( collideCol == collidedItem->col() ) ) ||
            ( ( request.m_direction.m_direction == Direction::left ) && ( collideCol == ( collidedItem->col() + collidedItem->width() - 1 ) ) ) ||
            ( ( request.m_direction.m_direction == Direction::down ) && ( collideRow == collidedItem->row() ) ) ||
            ( ( request.m_direction.m_direction == Direction::up ) && ( collideRow == ( collidedItem->row() + collidedItem->height() - 1 ) ) ) )
        {
            tuple[_edge] = 1;
        }
        else
        {
            tuple[_edge] = 0;
        }
        
        m_tupleRouter.route( tuple );
    }
}

void Compositor::performActionOnAll( enum ActionType actionType )
{
    Vector< SceneItem >::const_iterator iter;
    for( iter = m_currentScene.m_sceneItems.begin(); iter != m_currentScene.m_sceneItems.end(); ++iter )
    {
        const SceneItem& thisSceneItem( *iter );
        tryPerformAction( thisSceneItem, actionType );
    }
}

void Compositor::tryPerformAction( const SceneItem& sceneItem, enum ActionType actionType )
{
    String action( sceneItem.action() );
    Tokeniser lineTokeniser( action, ';' );
    String line( lineTokeniser.token() );
    while( line != "" )
    {
        Tokeniser actionTokeniser( line, ':' );
        String command( actionTokeniser.token() );
        String parameter( actionTokeniser.token() );
        if( actionType == loadAction && command == "load" && parameter != "" )
        {
            performAction( parameter, sceneItem );
        }
        else if( actionType == renderAction && command == "render" && parameter != "" )
        {
            performAction( parameter, sceneItem );
        }
        else if( actionType == bumpAction && command == "bump" && parameter != "" )
        {
            performAction( parameter, sceneItem );
        }

        line = lineTokeniser.token();
    }
}

void Compositor::performAction( const String& actionString, const SceneItem& sceneItem )
{
    Tokeniser actionTokeniser( actionString, ' ' );
    String action( actionTokeniser.token() );
    String parameter( actionTokeniser.token() );
    if( action == "play" && parameter != "" )
    {
        String loop( actionTokeniser.remainder() );
        m_midiPlayer.doPlay( m_coordinates, parameter, ( loop == _loop ) );
    }
    else if( action == "sign" && parameter != "" )
    {
        int border( ::atoi( parameter.c_str() ) );
        String colourWord( actionTokeniser.token() );
        String text( actionTokeniser.remainder() );

        char attributes( Terminal::attributes( colourWord ) );

        m_terminal.printFormatted( text,
                                   sceneItem.row() + border,
                                   sceneItem.col() + border,
                                   sceneItem.height() - ( border * 2 ),
                                   sceneItem.width() - ( border * 2 ),
                                   Terminal::hCentre,
                                   Terminal::noVCentre,
                                   attributes,
                                   Terminal::preserveBackground );

    }
    else if( action == "load" && parameter != "" )
    {
        if( m_programManager.load( m_programAssetLoaderFactory, m_coordinates, parameter ) )
        {
            m_programs.push_back( parameter );
        }
    }
    else if( action == "teleport" && parameter != "" )
    {
        // FIXME: Move into Coordinates class.
        // teleport "Lauren's World" 0N 0W 15,40
        // teleport "Lauren's World" 0N 0W
        // teleport 0N 0W 15,40
        // teleport 0N 0W

        String remainder( actionTokeniser.remainder() );
        if( !remainder.empty() ) parameter += " " + remainder;

        String worldName;
        Coordinates coordinates( m_coordinates );
        int row( -1 );
        int col( -1 );

        // Extract quoted world name, if any.
        size_t strStart( parameter.find_first_of( '"' ) );
        size_t strStop( String::npos );
        if( strStart != String::npos ) strStop = parameter.find_first_of( '"', strStart + 1 );
        if( ( strStart != String::npos ) && ( strStop != String::npos ) )
        {
            worldName = parameter.substr( strStart + 1, ( ( strStop - strStart ) - 1 ) );
            String worldID;
            if( m_worldbook.getWorldIDByName( worldName, worldID ) )
            {
                coordinates.m_worldID = worldID;
            }
        }

        // Find start of coords
        size_t coordsStart( 0 );
        if( ( strStop != String::npos ) && ( ( strStop + 2 ) < parameter.length() ) ) coordsStart = ( strStop + 2 );
        String coordsString( parameter.substr( coordsStart ) );

        // Extract coords
        Tokeniser coordsTokeniser( coordsString, ' ' );
        String yString( coordsTokeniser.token() );
        String xString( coordsTokeniser.token() );

        coordinates.m_x = ::atoi( xString.c_str() ); // Will ignore trailing non-digits.
        coordinates.m_y = ::atoi( yString.c_str() );

        if( xString.find( 'W' ) != String::npos ) coordinates.m_x *= -1;
        if( yString.find( 'S' ) != String::npos ) coordinates.m_y *= -1;

        // Extract row and column, if any.
        String rowColString( coordsTokeniser.token() );
        Tokeniser rowColTokeniser( rowColString, ',' );
        String rowString( rowColTokeniser.token() );
        String colString( rowColTokeniser.token() );
        if( !rowString.empty() && !colString.empty() )
        {
            row = ::atoi( rowString.c_str() );
            col = ::atoi( colString.c_str() );
        }

        m_teleportCoordinates = coordinates;
        m_teleportRow = row;
        m_teleportCol = col;
        m_teleportRequested = true;
    }
}

} // namespace World

} // namespace Agape
