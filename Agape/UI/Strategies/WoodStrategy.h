#ifndef AGAPE_UI_STRATEGIES_WOOD_H
#define AGAPE_UI_STRATEGIES_WOOD_H

#include "UI/Strategy.h"
#include "World/WorldCoordinates.h"
#include "World/WorldSummary.h"
#include "Collections.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace Timers
{
class Factory;
} // namespace Timers

namespace World
{
class Metadata;
class User;
class Utilities;
} // namespace World

using namespace World;

namespace WorldLoaders
{
class Factory;
} // namespace WorldLoaders

class AssetLoader;
class EntropySource;
class InputDevice;
class Terminal;
class Timer;
class WindowManager;
class Worldbook;

namespace UI
{

class Hotkeys;

namespace Strategies
{

class Wood : public Strategy
{
public:
    Wood( InputDevice& inputDevice,
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
          const String& navigationWindowName );
    ~Wood();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        none
    };

    enum Direction
    {
        up,
        down,
        left,
        right
    };

    void render();
    void tileBackground();
    int getGridNum();
    void drawWorld( int idx, int x, int y );
    void drawFlowers( int gridNum, int totalItems );
    
    void tryWalk( enum Direction direction, int newRow, int newCol, int glyphOffset1, int glyphOffset2 );

    bool inPool();
    bool canTeleport();

    void drawWorldSummary();

    void drawHotkeys();

    InputDevice& m_inputDevice;
    WorldLoaders::Factory& m_worldLoaderFactory;
    Utilities& m_worldUtilities;
    const Metadata& m_worldMetadata;
    const User& m_worldUser;
    const Worldbook& m_worldbook;
    WindowManager& m_windowManager;
    Hotkeys& m_hotkeys;
    String m_mapWindowName;
    String m_chatWindowName;
    String m_navigationWindowName;

    Timer* m_timer;
    
    bool m_completed;
    String m_nextStrategy;
    Value m_returnParameters;

    Terminal* m_mapTerminal;
    Terminal* m_chatTerminal;
    Terminal* m_navigationTerminal;

    int m_x;
    int m_y;

    int m_positionRow;
    int m_positionCol;
    bool m_positionHidden;
    int m_walkCycle;
    int m_glyphOffset;

    Vector< World::Summary > m_worldSummaries;

    int m_universeThings;

    Coordinates m_newCoordinates;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_STRATEGIES_WOOD_H
