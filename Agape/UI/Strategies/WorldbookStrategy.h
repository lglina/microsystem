#ifndef AGAPE_STRATEGIES_WORLDBOOK_H
#define AGAPE_STRATEGIES_WORLDBOOK_H

#include "UI/Strategy.h"
#include "Collections.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace World
{
class Utilities;
} // namespace World

class ConfigurationStore;
class Dialogue;
class InputDevice;
class Line;
class Terminal;
class WindowManager;
class Worldbook;

namespace UI
{

namespace Forms
{
class Form;
} // namespace Forms

namespace Strategies
{

class Worldbook : public Strategy
{
public:
    Worldbook( WindowManager& windowManager,
               const String& windowName,
               InputDevice& inputDevice,
               Agape::Worldbook& worldbook,
               World::Utilities& worldUtilities,
               ConfigurationStore& ConfigurationStore,
               Line& line,
               Dialogue& dialogue );

    ~Worldbook();

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
        select,
        chooseAvatarCreate,
        chooseAvatarJoin,
        create,
        join,
        enterWorld,
        loadJoined
    };

    void drawBackground();
    void drawSelectForm();

    void closeForm();

    bool getWorldID( String& worldID );
    
    void deleteWorld( const String& worldID );

    void setDefaultWorldID( const String& worldID );

    WindowManager& m_windowManager;
    String m_windowName;
    InputDevice& m_inputDevice;
    Agape::Worldbook& m_worldbook;
    World::Utilities& m_worldUtilities;
    ConfigurationStore& m_configurationStore;
    Line& m_line;
    Dialogue& m_dialogue;

    enum State m_state;

    bool m_completed;
    bool m_calling;
    Value m_callingParameters;
    String m_nextStrategy;

    Forms::Form* m_currentForm;
    Vector< String > m_worldIDs;

    Terminal* m_terminal;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_WORLDBOOK_H
