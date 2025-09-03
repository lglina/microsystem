#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "Assets/Asset.h"
#include "InputDevices/InputDevice.h"
#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "UI/Hotkeys.h"
#include "Utils/LiteStream.h"
#include "WorldLoaders/Factories/WorldLoadersFactory.h"
#include "WorldLoaders/WorldLoader.h"
#include "World/UniverseStats.h"
#include "World/User.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "World/WorldSummary.h"
#include "Collections.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"
#include "WoodStrategy.h"
#include "Worldbook.h"

#include <math.h>

using namespace Agape::InputDevices;
using namespace Agape::World;

namespace
{
    const char* backgroundAssetName( "grass-001" );
    const char* worldAssetName( "world-pool" );

    const int idleTimeout( 1000 ); // ms.

    enum Heights
    {
        background,
        foreground,
        ground
    };

    char treePositions[5][2] = { { 32, 8 }, { 8, 1 }, { 56, 1 }, { 8, 15 }, { 56, 15 } };
    char flowerPositions[18] = { 4, 17, 7, 9, 18, 12, 14, 13, 1, 16, 10, 3, 15, 2, 6, 5, 11, 8 };
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

Wood::Wood( InputDevice& inputDevice,
            WorldLoaders::Factory& worldLoaderFactory,
            Utilities& worldUtilities,
            const Metadata& worldMetadata,
            const User& worldUser,
            const Worldbook& worldbook,
            WindowManager& windowManager,
            Hotkeys& hotkeys,
            Timers::Factory& timerFactory,
            const String& mapWindowName,
            const String& chatWindowName,
            const String& navigationWindowName ) :
  m_inputDevice( inputDevice ),
  m_worldLoaderFactory( worldLoaderFactory ),
  m_worldUtilities( worldUtilities ),
  m_worldMetadata( worldMetadata ),
  m_worldUser( worldUser ),
  m_worldbook( worldbook ),
  m_windowManager( windowManager ),
  m_hotkeys( hotkeys ),
  m_mapWindowName( mapWindowName ),
  m_chatWindowName( chatWindowName ),
  m_navigationWindowName( navigationWindowName ),
  m_completed( false ),
  m_mapTerminal( nullptr ),
  m_chatTerminal( nullptr ),
  m_navigationTerminal( nullptr ),
  m_x( 0 ),
  m_y( 0 ),
  m_positionRow( 0 ),
  m_positionCol( 0 ),
  m_positionHidden( false ),
  m_walkCycle( 0 ),
  m_glyphOffset( 0 ),
  m_universeThings( 0 )
{
    m_timer = timerFactory.makeTimer();
}

Wood::~Wood()
{
    delete( m_timer );
}

void Wood::enter( const Value& parameters )
{
    m_completed = false;
    m_nextStrategy.clear();
    m_returnParameters = Value();

    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_mapWindowName, terminalWindow ) )
    {
        m_mapTerminal = terminalWindow.m_terminal;
    }
    else
    {
        m_completed = true;
    }

    if( m_windowManager.getTerminalWindow( m_chatWindowName, terminalWindow ) )
    {
        m_chatTerminal = terminalWindow.m_terminal;
    }
    else
    {
        m_completed = true;
    }

    if( m_windowManager.getTerminalWindow( m_navigationWindowName, terminalWindow ) )
    {
        m_navigationTerminal = terminalWindow.m_terminal;
    }
    else
    {
        m_completed = true;
    }

    if( !m_completed )
    {
        m_mapTerminal->clearScreen();
        render();

        // Try to position us for the world we just left. Note that this may
        // fail if the user is joined to > 5 worlds and didn't previously
        // depart here from within the screen containing their previous world
        // (as m_x and m_y are zero initially, so we may be on the
        // wrong screen).
        int idx( 0 );
        for( ; ( idx < 5 ) && ( idx < m_worldSummaries.size() ); ++idx )
        {
            if( m_worldSummaries[idx].m_worldID == m_worldMetadata.m_worldID )
            {
                m_positionRow = treePositions[idx][1] + 7;
                m_positionCol = treePositions[idx][0] + 11;
                break;
            }
        }

        if( idx == m_worldSummaries.size() )
        {
            m_positionRow = ( 4 * ( m_mapTerminal->height() / 5 ) );
            m_positionCol = m_mapTerminal->width() / 2;
        }

        m_mapTerminal->createCursor( m_worldUser.m_snowflake, m_positionRow, m_positionCol, m_worldUser.m_glyph, m_worldUser.m_attributes, Terminal::avatarCharset );

        drawWorldSummary();

        drawHotkeys();

        m_chatTerminal->clearScreen();
        m_chatTerminal->consumeString( "\r\nYou find yourself in a quiet place. Not much is happening here.\r\nI think if you so desired, you could choose to go on to somewhere else." );
    }
}

void Wood::returnTo( const Value& parameters )
{
}

bool Wood::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool Wood::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        m_mapTerminal->deleteCursor( m_worldUser.m_snowflake );
        
        if( m_mapTerminal ) m_mapTerminal->clearScreen();
        if( m_chatTerminal ) m_chatTerminal->clearScreen();
        if( m_navigationTerminal ) m_navigationTerminal->clearScreen();

        m_hotkeys.clear();

        nextStrategy = m_nextStrategy;
        parameters = m_returnParameters;

        return true;
    }

    return false;
}

void Wood::run()
{
    if( m_completed )
    {
        return; // In case terminals couldn't be found and are nullptr...
    }

    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == Key::carriageReturn ) // Eat all CRs.
        {
            continue;
        }
        else if( c == Key::up )
        {
            if( m_positionRow > 0 )
            {
                tryWalk( up, m_positionRow - 1, m_positionCol, 1, 2 );
            }
            else
            {
                ++m_y;
                m_positionRow = m_mapTerminal->height() - 1;
                render();
                m_mapTerminal->moveCursor( m_worldUser.m_snowflake, m_positionRow, m_positionCol );
            }
            drawWorldSummary();
        }
        else if( c == Key::down )
        {
            if( m_positionRow < ( m_mapTerminal->height() - 1 ) )
            {
                tryWalk( down, m_positionRow + 1, m_positionCol, 3, 4 );
            }
            else
            {
                --m_y;
                m_positionRow = 0;
                render();
                m_mapTerminal->moveCursor( m_worldUser.m_snowflake, m_positionRow, m_positionCol );
            }
            drawWorldSummary();
        }
        else if( c == Key::left )
        {
            if( m_positionCol > 0 )
            {
                tryWalk( left, m_positionRow, m_positionCol - 1, 5, 6 );
            }
            else
            {
                --m_x;
                m_positionCol = m_mapTerminal->width() - 1;
                render();
                m_mapTerminal->moveCursor( m_worldUser.m_snowflake, m_positionRow, m_positionCol );
            }
            drawWorldSummary();
        }
        else if( c == Key::right )
        {
            if( m_positionCol < ( m_mapTerminal->width() - 1 ) )
            {
                tryWalk( right, m_positionRow, m_positionCol + 1, 7, 8 );
            }
            else
            {
                ++m_x;
                m_positionCol = 0;
                render();
                m_mapTerminal->moveCursor( m_worldUser.m_snowflake, m_positionRow, m_positionCol );
            }
            drawWorldSummary();
        }
        else if( c == Key::newLine )
        {
            if( inPool() )
            {
                if( canTeleport() )
                {
                    if( m_newCoordinates.m_worldID != m_worldMetadata.m_worldID )
                    {
                        m_returnParameters[_doTeleport] = 1;
                        m_newCoordinates.toValue( m_returnParameters[_coordinates] );
                        // Now return to EnterWorld.
                    }
                    else
                    {
                        m_nextStrategy = "walk";
                        // Go back to current world
                    }
                    m_completed = true;
                }
                else
                {
                    m_chatTerminal->clearScreen();
                    m_chatTerminal->consumeString( "\r\n\r\nYou step into the water, but nothing happens." );
                }
            }
        }
        else if( c == Key::escape )
        {
            m_nextStrategy = "walk";
            m_completed = true;
            // Go back to current world
        }
    }

    if( m_timer->ms() >= idleTimeout )
    {
        if( m_glyphOffset != 0 )
        {
            m_glyphOffset = 0;

            if( !m_positionHidden )
            {
                m_mapTerminal->moveCursor( m_worldUser.m_snowflake, m_positionRow, m_positionCol, m_worldUser.m_glyph );
            }
        }
    }

    m_mapTerminal->run();
    m_chatTerminal->run();
    m_navigationTerminal->run();
}

void Wood::render()
{
    tileBackground();

    int gridNum( getGridNum() );

    WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );
    String reason;
    m_worldSummaries.clear();
    if( worldLoader->loadWorldSummaries( m_worldSummaries, gridNum * 5, 5, reason ) )
    {
        for( int idx = 0; ( idx < 5 ) && ( idx < m_worldSummaries.size() ); ++idx )
        {
            int x( 0 );
            int y( 0 );

            switch( idx )
            {
            case 0:
                x = treePositions[0][0] + ( gridNum % 2 == 0 );
                y = treePositions[0][1] + ( gridNum % 3 == 0 );
                break;
            case 1:
                x = treePositions[1][0] + ( gridNum % 5 == 0 );
                y = treePositions[1][1] + ( gridNum % 7 == 0 );
                break;
            case 2:
                x = treePositions[2][0] + ( gridNum % 11 == 0 );
                y = treePositions[2][1] + ( gridNum % 13 == 0 );
                break;
            case 3:
                x = treePositions[3][0] + ( gridNum % 17 == 0 );
                y = treePositions[3][1] + ( gridNum % 19 == 0 );
                break;
            case 4:
                x = treePositions[4][0] + ( gridNum % 23 == 0 );
                y = treePositions[4][1] + ( gridNum % 29 == 0 );
                break;
            default:
                break;
            }

            drawWorld( idx, x, y );
        }
    }

    UniverseStats universeStats;
    if( worldLoader->loadUniverseStats( universeStats, reason ) )
    {
        m_universeThings = universeStats.m_items;
    }

    drawFlowers( gridNum, m_universeThings );

    delete( worldLoader );
}

void Wood::tileBackground()
{
    AssetLoaders::Baked assetLoader( World::Coordinates(), backgroundAssetName );

    if( assetLoader.open() )
    {
        LOG_DEBUG( "Wood: Opened background tile" );
        Assets::ANSIFile ansiFile( assetLoader );

        int tileWidth = ansiFile.width();
        int tileHeight = ansiFile.height();
        for( int row = 0; row < m_mapTerminal->height(); row += tileHeight )
        {
            for( int col = 0; col < m_mapTerminal->width(); col += tileWidth )
            {
                LiteStream stream;
                stream << "Wood: Tiling at " << row << "," << col << " asset size " << ansiFile.dataSize() << " tile width " << tileWidth;
                LOG_DEBUG( stream.str() );
                m_mapTerminal->consumeNext( row, col );
                m_mapTerminal->consumeAsset( ansiFile, 0, ansiFile.dataSize(), tileWidth, col, Terminal::noMaxRow, Terminal::scrollLock );
            }
        }

        LOG_DEBUG( "Wood: Background tiling done" );
    }
}

int Wood::getGridNum()
{
    int longDim( 0 );
    int shortDim( 0 );
    int sideNumber( 0 );
    int gridBase( 0 );
    int sideDist( 0 );
    int gridNum( 0 );

    if( ( m_x != 0 ) || ( m_y != 0 ) )
    {
        if( abs( m_x ) > abs( m_y ) )
        {
            longDim = m_x;
            shortDim = m_y;
            if( m_x > 0 )
            {
                sideNumber = 1;
            }
            else if( m_x < 0 )
            {
                sideNumber = 3;
            }
        }
        else if( abs( m_x ) < abs( m_y ) )
        {
            longDim = m_y;
            shortDim = m_x;
            if( m_y > 0 )
            {
                sideNumber = 0;
            }
            else if( m_y < 0 )
            {
                sideNumber = 2;
            }
        }
        else
        {
            if( ( ( m_y > 0 ) && ( m_x < 0 ) ) || ( ( m_y < 0 ) && ( m_x > 0 ) ) )
            {
                longDim = m_y;
                shortDim = m_x;
                if( m_y > 0 )
                {
                    sideNumber = 0;
                }
                else if( m_y < 0 )
                {
                    sideNumber = 2;
                }
            }
            else
            {
                longDim = m_x;
                shortDim = m_y;
                if( m_x > 0 )
                {
                    sideNumber = 1;
                }
                else if( m_x < 0 )
                {
                    sideNumber = 3;
                }
            }
        }

        gridBase = pow( ( ( 2 * ( abs( longDim ) - 1 ) ) + 1 ), 2 );
        sideDist = abs( shortDim ) * 2;
        if( ( sideNumber == 1 ) || ( sideNumber == 2 ) )
        {
            if( shortDim > 0 ) --sideDist;
        }
        else
        {
            if( shortDim < 0 ) --sideDist;
        }
        gridNum = ( gridBase + sideNumber ) + ( sideDist * 4 );
    }

    LiteStream stream;
    stream << "Wood: x: " << m_x
           << " y: " << m_y
           << " long: " << longDim
           << " short: " << shortDim
           << " side: " << sideNumber
           << " gridBase: " << gridBase
           << " sideDist: " << sideDist
           << " GRIDNUM: " << gridNum;
    LOG_DEBUG( stream.str() );

    return gridNum;
}

void Wood::drawWorld( int idx, int x, int y )
{
    AssetLoaders::Baked assetLoader( World::Coordinates(), worldAssetName );

    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );

        m_mapTerminal->consumeNext( y, x );
        m_mapTerminal->consumeAsset( ansiFile, 0, ansiFile.dataSize(), ansiFile.width(), x, Terminal::noMaxRow, Terminal::scrollLock, Terminal::whitespaceTransparency );

        World::Summary& summary( m_worldSummaries[idx] );
        int numFlowers( 0 );
        if( summary.m_items < 25 ) numFlowers = 1;
        if( summary.m_items >= 25 ) numFlowers = 2;
        if( summary.m_items >= 50 ) numFlowers = 3;
        if( summary.m_items >= 100 ) numFlowers = 4;
        if( summary.m_items >= 200 ) numFlowers = 5;
        if( summary.m_items >= 500 ) numFlowers = 6;
        if( summary.m_items >= 1000 ) numFlowers = 7;
        if( summary.m_items >= 2000 ) numFlowers = 8;
        if( summary.m_items >= 5000 ) numFlowers = 9;
        if( summary.m_items >= 10000 ) numFlowers = 10;
        if( summary.m_items >= 50000 ) numFlowers = 11;
        if( summary.m_items >= 100000 ) numFlowers = 12;
        if( summary.m_items >= 500000 ) numFlowers = 13;
        if( summary.m_items >= 1000000 ) numFlowers = 14;
        if( summary.m_items >= 50000000 ) numFlowers = 15;
        if( summary.m_items >= 100000000 ) numFlowers = 16;
        if( summary.m_items >= 500000000 ) numFlowers = 17;
        if( summary.m_items >= 1000000000 ) numFlowers = 18;

        int flowerPos( 0 );
        for( int row = 0; row < ansiFile.height(); ++row )
        {
            for( int col = 0; col < ansiFile.width(); ++col )
            {
                char character;
                char attributes;
                m_mapTerminal->getBaseCharAt( row + y, col + x, character, attributes );
                if( character == ' ' )
                {
                    char thisPosFlowerNum( flowerPositions[flowerPos] );
                    if( numFlowers >= thisPosFlowerNum )
                    {
                        int colSeq( ( thisPosFlowerNum - 1 ) % 2 );
                        int colour( 0 );
                        if( colSeq == 0 )
                        {
                            colour = Terminal::colBrightRed;
                        }
                        else if( colSeq == 1 )
                        {
                            colour = Terminal::colBrightGreen;
                        }

                        char glyph( '\0' );
                        if( thisPosFlowerNum <= 4 )
                        {
                            glyph = '\x07';
                        }
                        else if( thisPosFlowerNum <= 10 )
                        {
                            glyph = '\x04';
                        }
                        else if( thisPosFlowerNum <= 17 )
                        {
                            glyph = '\x03';
                        }
                        else
                        {
                            glyph = '\x0F';
                        }

                        m_mapTerminal->consumeNext( row + y, col + x, colour );
                        m_mapTerminal->consumeChar( glyph, Terminal::scrollLock, Terminal::blit );
                    }

                    ++flowerPos;
                }
            }
        }
    }
}

void Wood::drawFlowers( int gridNum, int totalItems )
{
    int locIdx( gridNum * 100 );
    for( int row = 0; row < m_mapTerminal->height(); ++row )
    {
        for( int col = 0; col < m_mapTerminal->width() - 1; ++col )
        {
            // FIXME: Replace hard-coded primes with an algorithm?
            
            // LG: I have a simple and truly marvelous algorithm to create
            // primes in less than O(n) time - just haven't had a chance to put
            // it in here yet.

            if( ( ( totalItems < 1000 ) && ( ( locIdx % 263 ) == 0 ) ) ||
                ( ( totalItems >= 1000 ) && ( ( locIdx % 251 ) == 0 ) ) ||
                ( ( totalItems >= 2000 ) && ( ( locIdx % 239 ) == 0 ) ) ||
                ( ( totalItems >= 5000 ) && ( ( locIdx % 229 ) == 0 ) ) ||
                ( ( totalItems >= 10000 ) && ( ( locIdx % 223 ) == 0 ) ) ||
                ( ( totalItems >= 20000 ) && ( ( locIdx % 199 ) == 0 ) ) ||
                ( ( totalItems >= 50000 ) && ( ( locIdx % 193 ) == 0 ) ) ||
                ( ( totalItems >= 100000 ) && ( ( locIdx % 181 ) == 0 ) ) ||
                ( ( totalItems >= 200000 ) && ( ( locIdx % 173 ) == 0 ) ) ||
                ( ( totalItems >= 500000 ) && ( ( locIdx % 163 ) == 0 ) ) ||
                ( ( totalItems >= 1000000 ) && ( ( locIdx % 151 ) == 0 ) ) ||
                ( ( totalItems >= 2000000 ) && ( ( locIdx % 139 ) == 0 ) ) ||
                ( ( totalItems >= 5000000 ) && ( ( locIdx % 131 ) == 0 ) ) ||
                ( ( totalItems >= 10000000 ) && ( ( locIdx % 113 ) == 0 ) ) ||
                ( ( totalItems >= 20000000 ) && ( ( locIdx % 107 ) == 0 ) ) ||
                ( ( totalItems >= 50000000 ) && ( ( locIdx % 101 ) == 0 ) ) ||
                ( ( totalItems >= 100000000 ) && ( ( locIdx % 89 ) == 0 ) ) ||
                ( ( totalItems >= 1000000000 ) && ( ( locIdx % 79 ) == 0 ) ) )
            {
                char character1;
                char character2;
                char attribute1;
                char attribute2;
                m_mapTerminal->getBaseCharAt( row, col, character1, attribute1 );
                m_mapTerminal->getBaseCharAt( row, col, character2, attribute2 );
                if( ( character1 == '\xb1' ) && ( character2 == '\xb1' ) &&
                    ( attribute1 == 0x02 ) && ( attribute2 == 0x02 ) )
                {
                    int colSeq( locIdx % 4 );
                    int colour( 0 );
                    if( colSeq == 0 )
                    {
                        colour = Terminal::colBrightGreen;
                    }
                    else if( colSeq == 1 )
                    {
                        colour = Terminal::colBrightRed;
                    }
                    else if( colSeq == 2 )
                    {
                        colour = Terminal::colBrightMagenta;
                    }
                    else if( colSeq == 3 )
                    {
                        colour = Terminal::colYellow;
                    }

                    m_mapTerminal->consumeNext( row, col, Terminal::colGreen );
                    m_mapTerminal->consumeChar( '\xf4', Terminal::scrollLock, Terminal::blit );
                    m_mapTerminal->consumeNext( row, col + 1, colour );
                    m_mapTerminal->consumeChar( '\x0f', Terminal::scrollLock, Terminal::blit );
                }
            }

            ++locIdx;
        }
    }
}

void Wood::tryWalk( enum Direction direction, int newRow, int newCol, int glyphOffset1, int glyphOffset2 )
{
    char destCharacter;
    char destAttributes;
    m_mapTerminal->getBaseCharAt( newRow, newCol, destCharacter, destAttributes );
    unsigned char uattr( *( (unsigned char*)&destAttributes ) );
    int destfg( uattr & 0x0F );
    int destbg( uattr >> 4 );

    enum Heights height( background );
    if( ( destfg == Terminal::colRed ) &&
        ( destbg == Terminal::colGreen ) &&
        ( destCharacter == '\xb2' ) )
    {
        // Tree base
        height = ground;
    }
    else if( ( destfg == Terminal::colGreen ) &&
             ( destbg == Terminal::colRed ) &&
             ( destCharacter == '\xb0' ) )
    {
        // Tree trunk
        height = foreground;
    }
    else if( ( destfg == Terminal::colBlack ) &&
             ( destbg == Terminal::colGreen ) &&
             ( destCharacter == ' ' ) )
    {
        // Tree canopy
        height = foreground;
    }
    else if( ( destfg == Terminal::colBlack ) &&
             ( destbg == Terminal::colGreen ) &&
             ( destCharacter == '\xb0' ) )
    {
        // Tree canopy
        height = foreground;
    }

    if( height == background )
    {
        m_positionRow = newRow;
        m_positionCol = newCol;

        if( m_positionHidden )
        {
            m_positionHidden = false;
            if( m_walkCycle == 0 )
            {
                m_mapTerminal->createCursor( m_worldUser.m_snowflake, m_positionRow, m_positionCol, m_worldUser.m_glyph + glyphOffset1, m_worldUser.m_attributes, Terminal::avatarCharset );
                m_glyphOffset = glyphOffset1;
            }
            else
            {
                m_mapTerminal->createCursor( m_worldUser.m_snowflake, m_positionRow, m_positionCol, m_worldUser.m_glyph + glyphOffset2, m_worldUser.m_attributes, Terminal::avatarCharset );
                m_glyphOffset = glyphOffset2;
            }
        }
        else
        {
            if( m_walkCycle == 0 )
            {
                m_mapTerminal->moveCursor( m_worldUser.m_snowflake, m_positionRow, m_positionCol, m_worldUser.m_glyph + glyphOffset1 );
                m_glyphOffset = glyphOffset1;
            }
            else
            {
                m_mapTerminal->moveCursor( m_worldUser.m_snowflake, m_positionRow, m_positionCol, m_worldUser.m_glyph + glyphOffset2 );
                m_glyphOffset = glyphOffset2;
            }
        }

        if( m_walkCycle == 0 )
        {
            m_walkCycle = 1;
        }
        else
        {
            m_walkCycle = 0;
        }

        m_timer->reset();
    }
    else if( height == foreground )
    {
        m_positionRow = newRow;
        m_positionCol = newCol;
        m_positionHidden = true;
        m_mapTerminal->deleteCursor( m_worldUser.m_snowflake );
    }
    // else if ground (collision), don't move at all.
}

bool Wood::inPool()
{
    char currentCharacter;
    char currentAttributes;
    m_mapTerminal->getBaseCharAt( m_positionRow, m_positionCol, currentCharacter, currentAttributes );
    if( ( ( currentCharacter == '\xb0' ) || ( currentCharacter == '\xb2' ) || ( currentCharacter == '\xdb' ) ) &&
        ( currentAttributes == Terminal::colBrightBlue ) )
    {
        return true;
    }

    return false;
}

bool Wood::canTeleport()
{
    World::Metadata metadata;
    if( m_worldbook.getMetadata( m_newCoordinates.m_worldID, metadata ) )
    {
        return true;
    }

    return false;
}

void Wood::drawWorldSummary()
{
    int currentIdx( 0 );
    if( m_positionCol < ( 2 * ( m_mapTerminal->width() / 5 ) ) )
    {
        if( m_positionRow < ( m_mapTerminal->height() / 2 ) ) 
        {
            currentIdx = 1;
        }
        else
        {
            currentIdx = 3;
        }
    }
    else if( m_positionCol < ( 3 * ( m_mapTerminal->width() / 5 ) ) )
    {
        currentIdx = 0;
    }
    else
    {
        if( m_positionRow < ( m_mapTerminal->height() / 2 ) )
        {
            currentIdx = 2;
        }
        else
        {
            currentIdx = 4;
        }
    }

    String currentWorldStats;
    if( currentIdx < m_worldSummaries.size() )
    {
        LiteStream stream;

        World::Summary& summary( m_worldSummaries[currentIdx] );
        World::Metadata metadata;
        String worldIdent;
        if( m_worldbook.getMetadata( summary.m_worldID, metadata ) )
        {
            worldIdent = metadata.m_name;
        }
        else
        {
            worldIdent = summary.m_worldID;
        }

        if( worldIdent.length() < 17 )
        {
            stream << worldIdent;
        }
        else
        {
            stream << worldIdent.substr( 0, 17 ) << "...";
        }

        stream << ": " << summary.m_users << " user";
        if( summary.m_users != 1 ) stream << "s";
        stream << ", " << summary.m_items << " thing";
        if( summary.m_items != 1 ) stream << "s";
        stream << ".";

        currentWorldStats = stream.str();

        m_newCoordinates = Coordinates( summary.m_worldID );
    }

    String universeStats;
    {
        LiteStream stream;
        stream << "Universe: " << m_universeThings << " things.";
        universeStats = stream.str();
    }

    m_navigationTerminal->clearScreen();
    m_navigationTerminal->consumeNext( 0, 0 );
    m_navigationTerminal->consumeString( currentWorldStats, Terminal::scrollLock );
    
    m_navigationTerminal->consumeNext( 0, m_navigationTerminal->width() - universeStats.length() );
    m_navigationTerminal->consumeString( universeStats, Terminal::scrollLock );
}

void Wood::drawHotkeys()
{
    m_hotkeys.clear();
    m_hotkeys.show( "\x18\x19\x1B\x1A", "Walk" );
    m_hotkeys.show( "Ret", "Swim" );
    m_hotkeys.show( "Esc", "Depart" );
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
