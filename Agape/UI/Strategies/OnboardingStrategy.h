#ifndef AGAPE_STRATEGIES_ONBOARDING_H
#define AGAPE_STRATEGIES_ONBOARDING_H

#include "UI/Strategy.h"
#include "World/User.h"
#include "Collections.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace World
{
class Metadata;
class User;
} // namespace World

using namespace World;

class Clock;
class ConfigurationStore;
class InputDevice;
class Phonebook;
class Snowflake;
class Terminal;
class Value;
class WindowManager;
class Worldbook;

namespace UI
{

namespace Strategies
{

class Onboarding : public Strategy
{
public:
    enum State
    {
        none,
        welcome,
        enterAccountKey,
        enterDeviceKey,
        connectWifi,
        connect,
        createOrJoinWorld,
        chooseAvatarCreate,
        chooseAvatarJoin,
        createWorld,
        joinWorld,
        teleportDemo,
        joinLearning,
        addTeleportDemo,
        addTeleportHome,
        teleportDone,
        teleportDeclined,
        inviteFriend,
        inviteDeclined,
        completed
    };

    Onboarding( InputDevice& inputDevice,
                WindowManager& windowManager,
                Phonebook& phonebook,
                Worldbook& worldbook,
                const Metadata& worldMetadata,
                const User& worldUser,
                ConfigurationStore& configurationStore,
                Clock& clock,
                Snowflake& snowflake,
                const String& mainWindowName,
                const String& dialogueWindowName,
                const String& defaultPhoneName,
                const String& defaultPhoneNumber,
                const String& learningWorldID,
                const char* learningWorldKey );

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual bool needsFocus();

    virtual void run();

private:
    void drawMainBackground();
    void drawDialogueBackground( const String& title );

    void drawWelcome();
    void drawCreateOrJoinWorld();

    void drawTeleportDemo();
    void drawTeleportDone();
    void drawTeleportDeclined();

    void drawInviteFriend();
    void drawInviteDeclined();

    void showDialogue();
    void hideDialogue();

    InputDevice& m_inputDevice;
    WindowManager& m_windowManager;
    Phonebook& m_phonebook;
    Worldbook& m_worldbook;
    const Metadata& m_worldMetadata;
    const User& m_worldUser;
    ConfigurationStore& m_configurationStore;
    Clock& m_clock;
    Snowflake& m_snowflake;
    String m_mainWindowName;
    String m_dialogueWindowName;
    String m_defaultPhoneName;
    String m_defaultPhoneNumber;
    String m_learningWorldID;
    const char* m_learningWorldKey;

    enum State m_state;
    long long m_stateTime;
    bool m_calling;
    bool m_completed;
    Value m_callingParameters;
    Value m_returnParameters;
    String m_nextStrategy;

    bool m_needTeleport;

    Terminal* m_mainTerminal;
    Terminal* m_dialogueTerminal;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_ONBOARDING_H
