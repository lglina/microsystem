#ifndef AGAPE_UI_STRATEGIES_TELEPORT_H
#define AGAPE_UI_STRATEGIES_TELEPORT_H

#include "UI/Strategy.h"
#include "World/ScenePresence.h"
#include "World/Teleport.h"
#include "Collections.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace Encryptors
{
class Factory;
} // namespace Encryptors

namespace PresenceLoaders
{
class Factory;
} // namespace PresenceLoaders

namespace Timers
{
class Factory;
} // namespace Timers

namespace World
{
class Coordinates;
class Metadata;
class User;
} // namespace World

namespace WorldLoaders
{
class Factory;
} // namespace WorldLoaders

using namespace World;

class ConfigurationStore;
class Encryptor;
class InputDevice;
class Snowflake;
class Terminal;
class Timer;
class WindowManager;
class Worldbook;
class WorldLoader;

namespace UI
{

namespace Forms
{
class Form;
} // namespace Forms

class Dialogue;

namespace Strategies
{

class Teleport : public Strategy
{
public:
    Teleport( InputDevice& inputDevice,
              WorldLoaders::Factory& worldLoaderFactory,
              PresenceLoaders::Factory& presenceLoaderFactory,
              const User& worldUser,
              const Metadata& worldMetadata,
              const Coordinates& coordinates,
              const Worldbook& worldbook,
              ConfigurationStore& configurationStore,
              Encryptors::Factory& encryptorFactory,
              Snowflake& snowflake,
              WindowManager& windowManager,
              const String& windowName,
              Dialogue& dialogue,
              Timers::Factory& timerFactory );
    ~Teleport();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        none,
        list,
        listError,
        goError,
        add,
        addSuccess,
        syntaxError,
        addError,
        goTo,
        deleteSuccess,
        deleteError,
        setHomeSuccess,
        setHomeError
    };

    void drawBackground();
    void drawLoadPending();
    bool drawList();
    void drawListError();
    void drawGoError();
    bool drawAdd( bool here );
    void drawAddPending();
    void drawAddSuccess();
    void drawSyntaxError();
    void drawAddError();
    void drawGoTo();
    void drawDeletePending();
    void drawDeleteSuccess();
    void drawDeleteError();
    void drawSetHomePending();
    void drawSetHomeSuccess();
    void drawSetHomeError();
    void hideDialogue();

    void closeForm();

    void setHere();
    bool getTeleport();
    
    bool doTeleportCurrent();
    bool doTeleportPersonCurrent();
    bool doTeleportGoTo();
    bool doTeleport( const World::Teleport& teleport );
    
    bool addTeleport();
    bool deleteTeleport();
    bool setHome();

    InputDevice& m_inputDevice;
    WorldLoaders::Factory& m_worldLoaderFactory;
    PresenceLoaders::Factory& m_presenceLoaderFactory;
    const User& m_worldUser;
    const Metadata& m_worldMetadata;
    const Coordinates& m_coordinates;
    const Worldbook& m_worldbook;
    ConfigurationStore& m_configurationStore;
    Snowflake& m_snowflake;
    WindowManager& m_windowManager;
    String m_windowName;
    Dialogue& m_dialogue;

    Encryptor* m_encryptor;

    int m_currentRow;
    int m_currentCol;

    enum State m_state;
    enum State m_previousState;
    bool m_completed;
    Value m_returnParameters;
    Timer* m_timer;

    Terminal* m_terminal;
    Forms::Form* m_currentForm;
    Vector< String > m_worldListIDs;

    WorldLoader* m_worldLoader;
    Vector< World::Teleport > m_teleports;
    Vector< World::ScenePresence > m_worldPresences;

    World::Teleport m_newTeleport;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_STRATEGIES_TELEPORT_H
