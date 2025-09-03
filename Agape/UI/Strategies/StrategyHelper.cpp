#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "World/WorldCoordinates.h"
#include "StrategyHelper.h"
#include "String.h"
#include "Terminal.h"

namespace
{
    const char* backgroundAssetName( "menu-wide" );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

void Helper::drawMenuBackground( Terminal* terminal, const String& textAssetName )
{
    terminal->clearScreen();

    AssetLoaders::Baked backgroundAssetLoader( World::Coordinates(), backgroundAssetName );
    if( backgroundAssetLoader.open() )
    {
        Assets::ANSIFile ansiFile( backgroundAssetLoader );
        terminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
    }

    AssetLoaders::Baked textAssetLoader( World::Coordinates(), textAssetName );
    if( textAssetLoader.open() )
    {
        Assets::ANSIFile ansiFile( textAssetLoader );
        terminal->consumeNext( 1, 5 );
        terminal->consumeAsset( ansiFile,
                                0,
                                ansiFile.dataSize(),
                                ansiFile.width(),
                                5,
                                Terminal::noMaxRow,
                                Terminal::scrollLock,
                                Terminal::blit );
    }
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
