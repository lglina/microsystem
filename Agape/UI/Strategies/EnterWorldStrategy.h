#ifndef AGAPE_STRATEGIES_ENTER_WORLD_H
#define AGAPE_STRATEGIES_ENTER_WORLD_H

#include "UI/Strategy.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace World
{
class Metadata;
class User;
class Utilities;
} // namespace World

namespace WorldLoaders
{
class Factory;
} // namespace WorldLoaders

class ConfigurationStore;
class InputDevice;
class Line;
class Worldbook;

namespace UI
{

class Dialogue;

namespace Strategies
{

using namespace Agape::World;

class EnterWorld : public Strategy
{
public:
    EnterWorld( InputDevice& inputDevice,
                Worldbook& worldbook,
                Line& line,
                Metadata& sessionMetadata,
                User& sessionUser,
                World::Utilities& worldUtilities,
                WorldLoaders::Factory& worldLoaderFactory,
                ConfigurationStore& configurationStore,
                Dialogue& dialoogue );

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        none,
        connect,
        pending,
        walk,
        error
    };

    bool getHomeLocation();
    bool getDefaultWorld();

    bool enterWorld( const String& worldID, User& sessionUser, Metadata& sessionMetadata );

    void drawPending();
    void drawErrorNoDefault();
    void drawErrorEnter();
    void hideDialogue();

    InputDevice& m_inputDevice;
    Worldbook& m_worldbook;
    Line& m_line;
    Metadata& m_sessionMetadata;
    User& m_sessionUser;
    World::Utilities& m_worldUtilities;
    WorldLoaders::Factory& m_worldLoaderFactory;
    ConfigurationStore& m_configurationStore;
    Dialogue& m_dialogue;

    Value m_parameters;
    enum State m_state;
    bool m_completed;
    bool m_calling;
    String m_nextStrategy;
    Value m_callingParameters;

    String m_worldID;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_ENTER_WORLD_H
