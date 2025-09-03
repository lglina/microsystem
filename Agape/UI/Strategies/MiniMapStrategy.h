#ifndef AGAPE_STRATEGIES_MINI_MAP_H
#define AGAPE_STRATEGIES_MINI_MAP_H

#include "UI/Strategy.h"
#include "World/WorldCoordinates.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace AssetLoaders
{
class Factory;
} // namespace AssetLoaders

using namespace World;

class InputDevice;
class Terminal;
class WindowManager;

namespace UI
{

class Dialogue;
class Hotkeys;
class Navigation;

namespace Strategies
{

class MiniMap : public Strategy
{
public:
    MiniMap( AssetLoaders::Factory& assetLoaderFactory,
             Coordinates& coordinates,
             Navigation& navigation,
             InputDevice& inputDevice,
             Hotkeys& hotkeys,
             WindowManager& windowManager,
             const String& windowName,
             Dialogue& dialogue );
    virtual ~MiniMap();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    void purgeCache();
    void drawMaps();
    void drawGrid( int gridx, int gridy, bool selected );

    World::Coordinates coordsAtOffset( int x, int y );

    void drawHotkeys();

    AssetLoaders::Factory& m_assetLoaderFactory;
    Coordinates& m_coordinates;
    Navigation& m_navigation;
    InputDevice& m_inputDevice;
    Hotkeys& m_hotkeys;
    WindowManager& m_windowManager;
    String m_windowName;
    Dialogue& m_dialogue;

    Value m_returnParameters;
    bool m_completed;

    Terminal* m_terminal;

    Coordinates m_centreCoordinates;

    int m_xtiles;
    int m_ytiles;

    int m_xwidth;
    int m_yheight;
    
    int m_xborder;
    int m_yborder;

    int m_cursorx;
    int m_cursory;

    Coordinates m_cacheCoordinates;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_MINI_MAP_H
