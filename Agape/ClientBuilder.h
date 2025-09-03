#ifndef AGAPE_CLIENT_BUILDER_H
#define AGAPE_CLIENT_BUILDER_H

#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "Audio/MIDIPlayer.h"
#include "Clocks/Clock.h"
#include "EditorFactory.h"
#include "Encryptors/Factories/BlockEncryptorsFactory.h"
#include "Encryptors/Factories/EncryptorsFactory.h"
#include "Encryptors/Hash.h"
#include "EntropySources/EntropySource.h"
#include "EventClocks/EventClock.h"
#include "GraphicsDrivers/GraphicsDriver.h"
#include "GraphicsDrivers/Headless.h"
#include "Highlighters/Factories/HighlighterFactory.h"
#include "InputDevices/InputDevice.h"
#include "Lines/Line.h"
#include "Memories/Memory.h"
#include "NativeActors/EntropyActor.h"
#include "NativeActors/EventTimerActor.h"
#include "NativeActors/Musician.h"
#include "NativeActors/PlatformActor.h"
#include "NativeActors/SnowflakeActor.h"
#include "NativeActors/ThingActor.h"
#include "NativeActors/UserActor.h"
#include "NativeActors/WorldActor.h"
#include "Platforms/Platform.h"
#include "PresenceLoaders/Factories/PresenceLoadersFactory.h"
#include "SceneLoaders/Factories/SceneLoadersFactory.h"
#include "TelegramLoaders/Factories/TelegramLoadersFactory.h"
#include "TelegramLoaders/TelegramLoader.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/NullTimer.h"
#include "TupleRoutes/TupleRoute.h"
#include "UI/Dialogue.h"
#include "UI/Hotkeys.h"
#include "UI/Navigation.h"
#include "UI/NotificationsUI.h"
#include "UI/PlatformUI.h"
#include "UI/PresenceUI.h"
#include "UI/Strategy.h"
#include "UI/TabBar.h"
#include "UI/VRTime.h"
#include "Utils/Snowflake.h"
#include "World/Compositor.h"
#include "World/MiniMap.h"
#include "World/User.h"
#include "World/WorldMetadata.h"
#include "World/WorldUtilities.h"
#include "WorldLoaders/Factories/WorldLoadersFactory.h"
#include "ANSIEditorFactory.h"
#include "ANSITerminal.h"
#include "Chat.h"
#include "Chooser.h"
#include "Client.h"
#include "ConfigurationStore.h"
#include "ConnectionMonitor.h"
#include "FunctionDispatcher.h"
#include "KeyUtilities.h"
#include "Phonebook.h"
#include "ProgramManager.h"
#include "RWBuffer.h"
#include "Session.h"
#include "String.h"
#include "Terminal.h"
#include "TupleDispatcher.h"
#include "TupleRouter.h"
#include "WindowManager.h"
#include "Worldbook.h"

namespace Agape
{

// Construction: Base c'tor calls base setMembersNull(),
//               derived c'tor cals derived _setMembersNull()

// Unbuilding: Base unbuild() calls derived _unbuild() which calls derived
//             _deleteMembers() and _setMembersNull().
//             Then base unbuild() calls base
//             deleteMembers() and setMembersNull().

// Destruction: Derived d'tor->_deleteMembers(), Base d'tor->deleteMembers().

class ClientBuilder
{
public:
    ClientBuilder();
    virtual ~ClientBuilder();

    virtual Client* build( Chooser& chooser );
    void unbuild();

protected:
    virtual void _unbuild() {};

    virtual void getMachineID() = 0;
    virtual void buildTimerFactory() = 0;
    virtual void buildPerformanceTimerFactory() = 0;
    virtual void buildConfigurationStore() = 0;
    virtual void buildGraphicsDriver() = 0;
    virtual void buildPlatform() = 0;
    virtual void buildLineDriver() = 0;
    virtual void buildLine() = 0;
    virtual void buildInputDevice() = 0;
    virtual void buildTupleRoute() = 0;
    virtual void buildClock() = 0;
    virtual void buildEntropySource() = 0;
    virtual void buildAssetLoaderFactory() = 0;
    virtual void buildProgramAssetLoaderFactory() = 0;
    virtual void buildTelegramAssetLoaderFactory() = 0;
    virtual void buildMIDIAssetLoaderFactory() = 0;
    virtual void buildSceneLoaderFactory() = 0;
    virtual void buildPresenceLoaderFactory() = 0;
    virtual void buildTelegramLoaderFactory() = 0;
    virtual void buildMIDIPlayer() = 0;
    virtual void buildWorldLoaderFactory() = 0;
    virtual void buildSplash() = 0;
    virtual void buildMiniMapAssetLoaderFactory() = 0;

    int m_machineID;
    Timers::Factory* m_timerFactory;
    Timers::Factory* m_performanceTimerFactory;
    ConfigurationStore* m_configurationStore;
    GraphicsDriver* m_graphicsDriver;
    WindowManager* m_windowManager;
    UI::Dialogue* m_dialogue;
    RWBuffer* m_rwBuffer;
    LineDriver* m_lineDriver;
    Line* m_line;
    InputDevice* m_inputDevice;
    Agape::Linda2::TupleDispatcher* m_tupleDispatcher;
    Agape::Linda2::TupleRoute* m_tupleRoute;
    Agape::Linda2::TupleRouter* m_tupleRouter;
    Carlo::FunctionDispatcher* m_functionDispatcher;
    UI::VRTime* m_vrTime;
    Clock* m_clock;
    EventClock* m_eventClock;
    Snowflake* m_snowflake;
    Carlo::ProgramManager* m_programManager;
    EntropySource* m_entropySource;
    Encryptors::Factory* m_encryptorFactory;
    Encryptors::BlockFactory* m_blockEncryptorFactory;
    Hash* m_hash;
    World::Metadata* m_worldMetadata;
    World::User* m_worldUser;
    AssetLoaders::Factory* m_assetLoaderFactory;
    AssetLoaders::Factory* m_programAssetLoaderFactory;
    AssetLoaders::Factory* m_telegramAssetLoaderFactory;
    AssetLoaders::Factory* m_midiAssetLoaderFactory;
    SceneLoaders::Factory* m_sceneLoaderFactory;
    PresenceLoaders::Factory* m_presenceLoaderFactory;
    TelegramLoaders::Factory* m_telegramLoaderFactory;
    Audio::MIDIPlayer* m_midiPlayer;
    Platform* m_platform;
    World::Utilities* m_worldUtilities;
    WorldLoaders::Factory* m_worldLoaderFactory;
    UI::Strategy* m_splash;
    UI::Strategy* m_memoryStrategy;
    UI::Strategy* m_testStrategy;
    World::MiniMap* m_miniMap;
    AssetLoaders::Factory* m_miniMapAssetLoaderFactory;

private:
    void deleteMembers();
    void setMembersNull();

    Timer* m_animationTimer;
    GraphicsDrivers::Headless* m_headlessGraphicsDriver;
    Timers::Null* m_nullTimer;
    ANSITerminal* m_drawTerminal;
    Terminal* m_mapTerminal;
    Terminal* m_navigationTerminal;
    Terminal* m_statusTerminal;
    Terminal* m_hotkeysTerminal;
    Terminal* m_presenceTerminal;
    Terminal* m_chatTerminal;
    Terminal* m_largeDialogueTerminal;
    Terminal* m_smallDialogueTerminal;
    Terminal* m_errorsTerminal;
    Terminal* m_linda2Terminal;
    World::Coordinates* m_coordinates;
    World::Compositor* m_compositor;
    Phonebook* m_phonebook;
    Worldbook* m_worldbook;
    KeyUtilities* m_keyUtilities;
    Agape::Editor::Highlighters::Factory* m_highlighterFactory;
    Agape::Editor::Factory* m_editorFactory;
    Agape::Editor::Factory* m_telegramEditorFactory;
    Agape::Editor::Factory* m_sceneItemTextEditorFactory;
    Agape::ANSIEditor::Factory* m_ansiEditorFactory;
    Chat* m_chat;
    UI::TabBar* m_tabBar;
    UI::Hotkeys* m_hotkeys;
    UI::Navigation* m_navigation;
    UI::Presence* m_presence;
    UI::PlatformUI* m_platformUI;
    UI::NotificationsUI* m_notificationsUI;
    UI::Strategy* m_connectWiFi;
    UI::Strategy* m_connect;
    UI::Strategy* m_onboarding;
    UI::Strategy* m_menu;
    UI::Strategy* m_settings;
    UI::Strategy* m_modemConfig;
    UI::Strategy* m_phonebookStrategy;
    UI::Strategy* m_worldbookStrategy;
    UI::Strategy* m_chooseAvatar;
    UI::Strategy* m_createWorldStrategy;
    UI::Strategy* m_joinWorldStrategy;
    UI::Strategy* m_enterWorldStrategy;
    UI::Strategy* m_walk;
    UI::Strategy* m_editWorld;
    UI::Strategy* m_carlo;
    UI::Strategy* m_linda2;
    UI::Strategy* m_enterKeyStrategy;
    UI::Strategy* m_telaStrategy;
    UI::Strategy* m_telegramStrategy;
    UI::Strategy* m_teleportStrategy;
    UI::Strategy* m_woodStrategy;
    UI::Strategy* m_inviteFriendStrategy;
    UI::Strategy* m_miniMapStrategy;
    UI::Strategy* m_creditsStrategy;
    UI::Strategy* m_ansiEditorStrategy;
    UI::Strategy* m_messageStrategy;
    Agape::Linda2::Actors::NativeActors::World* m_worldActor;
    Agape::Linda2::Actors::NativeActors::Musician* m_musician;
    Agape::Linda2::Actors::NativeActors::Snowflake* m_snowflakeActor;
    Agape::Linda2::Actors::NativeActors::Thing* m_thingActor;
    Agape::Linda2::Actors::NativeActors::User* m_userActor;
    Agape::Linda2::Actors::NativeActors::Platform* m_platformActor;
    Agape::Linda2::Actors::NativeActors::Entropy* m_entropyActor;
    Agape::Linda2::Actors::NativeActors::EventTimer* m_eventTimerActor;

    Session* m_session;
    ConnectionMonitor* m_connectionMonitor;

    Client* m_client;
};

}

#endif // AGAPE_CLIENT_BUILDER_H
