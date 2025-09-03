#ifndef AGAPE_WORLD_MINI_MAP_H
#define AGAPE_WORLD_MINI_MAP_H

#include "AssetLoaders/Factories/BakedAssetLoaderFactory.h"
#include "GraphicsDrivers/Headless.h"
#include "Timers/NullTimer.h"
#include "ANSITerminal.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{
class Factory;
} // AssetLoaders

namespace SceneLoaders
{
class Factory;
} // SceneLoaders

namespace World
{

class Coordinates;

class MiniMap
{
public:
    MiniMap( SceneLoaders::Factory& sceneLoaderFactory,
             AssetLoaders::Factory& assetLoaderFactory,
             ANSITerminal& renderTerminal );
    ~MiniMap();

    void render( const World::Coordinates& coordinates, int xtiles, int ytiles );

    int renderedSize();
    int read( char* data, int offset, int len );

private:
    void tileBackground( const World::Coordinates& coordinates );
    void getAssetFlags( const String& commentBlock, bool& blit, bool& animate, int& frames );

    SceneLoaders::Factory& m_sceneLoaderFactory;
    AssetLoaders::Factory& m_assetLoaderFactory;
    ANSITerminal& m_renderTerminal;

    int m_renderSize;
    char* m_buffer;

    AssetLoaders::Factories::Baked m_unknownAssetLoaderFactory;

    bool m_fastMode;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_WORLD_MINI_MAP_H
