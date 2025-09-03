#include "Encryptors/Factories/AESBlockEncryptorFactory.h"
#include "Encryptors/Factories/AESEncryptorFactory.h"
#include "Encryptors/SHA256/SHA256Hash.h"
#include "EventClocks/EventClock.h"
#include "GraphicsDrivers/GraphicsDriver.h"
#include "GraphicsDrivers/Headless.h"
#include "Highlighters/Factories/CarloHighlighterFactory.h"
#include "InputDevices/InputDevice.h"
#include "LineDrivers/LineDriver.h"
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
#include "TelegramLoaders/TelegramLoader.h"
#include "Timers/NullTimer.h"
#include "Timers/Timer.h"
#include "UI/Strategies/ANSIEditorStrategy.h"
#include "UI/Strategies/CarloStrategy.h"
#include "UI/Strategies/ChooseAvatar.h"
#include "UI/Strategies/ConnectStrategy.h"
#include "UI/Strategies/ConnectWiFi.h"
#include "UI/Strategies/CreateWorld.h"
#include "UI/Strategies/Credits.h"
#include "UI/Strategies/EditWorldStrategy.h"
#include "UI/Strategies/EnterKeyStrategy.h"
#include "UI/Strategies/EnterWorldStrategy.h"
#include "UI/Strategies/InviteFriendStrategy.h"
#include "UI/Strategies/JoinWorld.h"
#include "UI/Strategies/Linda2Strategy.h"
#include "UI/Strategies/Menu.h"
#include "UI/Strategies/MessageStrategy.h"
#include "UI/Strategies/MiniMapStrategy.h"
#include "UI/Strategies/ModemConfigStrategy.h"
#include "UI/Strategies/OnboardingStrategy.h"
#include "UI/Strategies/PhonebookStrategy.h"
#include "UI/Strategies/SettingsStrategy.h"
#include "UI/Strategies/TelaStrategy.h"
#include "UI/Strategies/TelegramStrategy.h"
#include "UI/Strategies/TeleportStrategy.h"
#include "UI/Strategies/WalkStrategy.h"
#include "UI/Strategies/WoodStrategy.h"
#include "UI/Strategies/WorldbookStrategy.h"
#include "UI/Dialogue.h"
#include "UI/Hotkeys.h"
#include "UI/Navigation.h"
#include "UI/NotificationsUI.h"
#include "UI/PlatformUI.h"
#include "UI/PresenceUI.h"
#include "UI/TabBar.h"
#include "Utils/Cartesian.h"
#include "Utils/Snowflake.h"
#include "Utils/StrToHex.h"
#include "WorldLoaders/Factories/WorldLoadersFactory.h"
#include "World/Compositor.h"
#include "World/MiniMap.h"
#include "World/User.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "World/WorldUtilities.h"
#include "ANSITerminal.h"
#include "Chat.h"
#include "Chooser.h"
#include "Client.h"
#include "ClientBuilder.h"
#include "ConfigurationStore.h"
#include "ConnectionMonitor.h"
#include "EditorFactory.h"
#include "FunctionDispatcher.h"
#include "KeyUtilities.h"
#include "ProgramManager.h"
#include "ReadableWritable.h"
#include "RWBuffer.h"
#include "Session.h"
#include "String.h"
#include "TupleDispatcher.h"
#include "TupleRouter.h"
#include "Warp.h"
#include "WindowManager.h"
#include "Worldbook.h"

#include "Loggers/Logger.h"

namespace Agape
{

ClientBuilder::ClientBuilder()
{
    setMembersNull();
}

ClientBuilder::~ClientBuilder()
{
    LOG_DEBUG( "ClientBuilder: Destructing" );
    deleteMembers();
}

Client* ClientBuilder::build( Chooser& chooser )
{
    getMachineID();

    LOG_DEBUG( "Building timer" );
    buildTimerFactory();
    ReadableWritable::setTimerFactory( m_timerFactory );
    buildPerformanceTimerFactory();
    Warp::setTimerFactory( m_performanceTimerFactory );

    LOG_DEBUG( "Building configuration store" );
    buildConfigurationStore();

    LOG_DEBUG( "Building graphics driver" );
    buildGraphicsDriver();

    LOG_DEBUG( "Building windows and terminals" );
    m_windowManager = new WindowManager( *m_graphicsDriver );

    m_animationTimer = m_timerFactory->makeTimer();

    m_headlessGraphicsDriver = new GraphicsDrivers::Headless;
    m_nullTimer = new Timers::Null;
    m_drawTerminal = new ANSITerminal( 80, 25, String(), *m_headlessGraphicsDriver, *m_nullTimer );

    m_mapTerminal = new ANSITerminal( 80, 25, _Map, *m_graphicsDriver, *m_animationTimer, m_drawTerminal );
    m_navigationTerminal = new ANSITerminal( 80, 1, _Navigation, *m_graphicsDriver, *m_animationTimer );
    m_statusTerminal = new ANSITerminal( 80, 1, _Status, *m_graphicsDriver, *m_animationTimer );
    m_hotkeysTerminal = new ANSITerminal( 10, 25, _Hotkeys, *m_graphicsDriver, *m_animationTimer, nullptr, false );
    m_presenceTerminal = new ANSITerminal( 10, 25, _Presence, *m_graphicsDriver, *m_animationTimer, nullptr, false );
    m_chatTerminal = new ANSITerminal( 80, 10, _Chat, *m_graphicsDriver, *m_animationTimer );
    m_largeDialogueTerminal = new ANSITerminal( 53, 17, _LargeDialogue, *m_graphicsDriver, *m_animationTimer );
    m_smallDialogueTerminal = new ANSITerminal( 40, 8, _SmallDialogue, *m_graphicsDriver, *m_animationTimer, nullptr, false );
    m_errorsTerminal = new ANSITerminal( 80, 2, _Errors, *m_graphicsDriver, *m_animationTimer, nullptr, false );
    m_linda2Terminal = new ANSITerminal( 80, 12, _Linda2, *m_graphicsDriver, *m_animationTimer );

    // NB: The order of window creation is significant. Lower priority windows
    // MUST be first (e.g. Map before Asset), as any redraw of visible windows
    // will occur in order of first to last in this list.
    {
    GraphicsDriver::Window window;
    window.m_name = _Map;
    window.m_rect = Rectangle( 80, 48, 400, 640 );
    window.m_visible = true;
    WindowManager::TerminalWindow terminalWindow;
    terminalWindow.m_terminal = m_mapTerminal;
    terminalWindow.m_window = m_graphicsDriver->createWindow( window );
    m_windowManager->createTerminalWindow( terminalWindow );
    }

    {
    GraphicsDriver::Window window;
    window.m_name = _Navigation;
    window.m_rect = Rectangle( 80, 448, 16, 640 );
    window.m_visible = true;
    WindowManager::TerminalWindow terminalWindow;
    terminalWindow.m_terminal = m_navigationTerminal;
    terminalWindow.m_window = m_graphicsDriver->createWindow( window );
    m_windowManager->createTerminalWindow( terminalWindow );
    }

    {
    GraphicsDriver::Window window;
    window.m_name = _Status;
    window.m_rect = Rectangle( 80, 464, 16, 640 );
    window.m_visible = true;
    WindowManager::TerminalWindow terminalWindow;
    terminalWindow.m_terminal = m_statusTerminal;
    terminalWindow.m_window = m_graphicsDriver->createWindow( window );
    m_windowManager->createTerminalWindow( terminalWindow );
    }

    {
    GraphicsDriver::Window window;
    window.m_name = _Hotkeys;
    window.m_rect = Rectangle( 0, 48, 400, 80 );
    window.m_visible = true;
    WindowManager::TerminalWindow terminalWindow;
    terminalWindow.m_terminal = m_hotkeysTerminal;
    terminalWindow.m_window = m_graphicsDriver->createWindow( window );
    m_windowManager->createTerminalWindow( terminalWindow );
    }

    {
    GraphicsDriver::Window window;
    window.m_name = _Presence;
    window.m_rect = Rectangle( 720, 48, 400, 80 );
    window.m_visible = true;
    WindowManager::TerminalWindow terminalWindow;
    terminalWindow.m_terminal = m_presenceTerminal;
    terminalWindow.m_window = m_graphicsDriver->createWindow( window );
    m_windowManager->createTerminalWindow( terminalWindow );
    }

    {
    GraphicsDriver::Window window;
    window.m_name = _Chat;
    window.m_rect = Rectangle( 80, -112, 160, 640 );
    window.m_visible = true;
    WindowManager::TerminalWindow terminalWindow;
    terminalWindow.m_terminal = m_chatTerminal;
    terminalWindow.m_window = m_graphicsDriver->createWindow( window );
    m_windowManager->createTerminalWindow( terminalWindow );
    }

    {
    GraphicsDriver::Window window;
    window.m_name = _LargeDialogue;
    window.m_rect = Rectangle( 184, 112, 272, 424 );
    window.m_visible = false;
    WindowManager::TerminalWindow terminalWindow;
    terminalWindow.m_terminal = m_largeDialogueTerminal;
    terminalWindow.m_window = m_graphicsDriver->createWindow( window );
    m_windowManager->createTerminalWindow( terminalWindow );
    }

    {
    GraphicsDriver::Window window;
    window.m_name = _SmallDialogue;
    window.m_rect = Rectangle( 240, 168, 128, 320 );
    window.m_visible = false;
    WindowManager::TerminalWindow terminalWindow;
    terminalWindow.m_terminal = m_smallDialogueTerminal;
    terminalWindow.m_window = m_graphicsDriver->createWindow( window );
    m_windowManager->createTerminalWindow( terminalWindow );
    }

    {
    GraphicsDriver::Window window;
    window.m_name = _Errors;
    window.m_rect = Rectangle( 80, 448, 32, 640 );
    window.m_visible = false;
    WindowManager::TerminalWindow terminalWindow;
    terminalWindow.m_terminal = m_errorsTerminal;
    terminalWindow.m_window = m_graphicsDriver->createWindow( window );
    m_windowManager->createTerminalWindow( terminalWindow );
    }

    {
    GraphicsDriver::Window window;
    window.m_name = _Linda2;
    window.m_rect = Rectangle( 80, 288, 192, 640 );
    window.m_visible = false;
    WindowManager::TerminalWindow terminalWindow;
    terminalWindow.m_terminal = m_linda2Terminal;
    terminalWindow.m_window = m_graphicsDriver->createWindow( window );
    m_windowManager->createTerminalWindow( terminalWindow );
    }

    m_dialogue = new UI::Dialogue( *m_windowManager,
                                   _SmallDialogue,
                                   _small_dialogue,
                                   _small_dialogue_g,
                                   _error );

    LOG_DEBUG( "Building platform" );
    buildPlatform();

    LOG_DEBUG( "Building line driver" );
    buildLineDriver();

    LOG_DEBUG( "Building line" );
    buildLine();

    LOG_DEBUG( "Building input device" );    
    buildInputDevice();

    LOG_DEBUG( "Building tuple dispatcher" );
    m_tupleDispatcher = new Agape::Linda2::TupleDispatcher;

    LOG_DEBUG( "Building tuple route" );
    buildTupleRoute();

    LOG_DEBUG( "Building tuple router" );
    m_tupleRouter = new Agape::Linda2::TupleRouter( *m_tupleDispatcher, uintToHex( m_machineID ), *m_timerFactory );
    m_tupleRouter->addRoute( m_tupleRoute );
    m_tupleRouter->setMyID( uintToHex( m_machineID ) );

    LOG_DEBUG( "Building function dispatcher" );
    m_functionDispatcher = new Carlo::FunctionDispatcher;

    LOG_DEBUG( "Building clock" );
    m_vrTime = new UI::VRTime( *m_windowManager, // Build before clock, in case we're using Clocks::VRTime.
                               _Navigation,
                               *m_tupleRouter,
                               *m_functionDispatcher );
    buildClock();

    LOG_DEBUG( "Building snowflake" );
    m_snowflake = new Snowflake( *m_clock, m_machineID );

    LOG_DEBUG( "Building program manager" );
    m_programManager = new Carlo::ProgramManager( *m_tupleRouter, *m_functionDispatcher );

    buildEntropySource();
    m_encryptorFactory = new Encryptors::Factories::AES( *m_entropySource );
    m_blockEncryptorFactory = new Encryptors::Factories::AESBlock( *m_entropySource );
    m_hash = new Hashes::SHA256;

    m_coordinates = new World::Coordinates;
    m_worldMetadata = new World::Metadata;
    m_worldUser = new World::User;

    LOG_DEBUG( "Building asset loader factory" );
    buildAssetLoaderFactory();

    LOG_DEBUG( "Building program asset loader factory" );
    buildProgramAssetLoaderFactory();

    LOG_DEBUG( "Building telegram asset loader factory" );
    buildTelegramAssetLoaderFactory();

    LOG_DEBUG( "Building MIDI asset loader factory" );
    buildMIDIAssetLoaderFactory();

    LOG_DEBUG( "Building scene loader factory" );
    buildSceneLoaderFactory();

    LOG_DEBUG( "Building presence loader factory" );
    buildPresenceLoaderFactory();

    LOG_DEBUG( "Building telegram loader factory" );
    buildTelegramLoaderFactory();

    LOG_DEBUG( "Building MIDI player" );
    buildMIDIPlayer();

    m_phonebook = new Phonebook( *m_configurationStore );
    m_worldbook = new Worldbook( *m_configurationStore );

    LOG_DEBUG( "Building compositor" );
    m_compositor = new World::Compositor( *m_mapTerminal,
                                          *m_sceneLoaderFactory,
                                          *m_assetLoaderFactory,
                                          *m_presenceLoaderFactory,
                                          *m_worldbook,
                                          *m_tupleRouter,
                                          *m_programManager,
                                          *m_programAssetLoaderFactory,
                                          *m_timerFactory,
                                          *m_clock,
                                          *m_midiPlayer );

    m_keyUtilities = new KeyUtilities( *m_entropySource,
                                       *m_encryptorFactory,
                                       *m_hash );
    m_worldUtilities = new World::Utilities( *m_keyUtilities,
                                             *m_snowflake,
                                             *m_encryptorFactory );

    buildWorldLoaderFactory();

    m_highlighterFactory = new Agape::Editor::Highlighters::Factories::Carlo( *m_tupleRouter,
                                                                              *m_functionDispatcher,
                                                                              *m_programManager );

    m_editorFactory = new Agape::Editor::Factory( *m_mapTerminal,
                                                  *m_programAssetLoaderFactory,
                                                  m_highlighterFactory,
                                                  *m_timerFactory,
                                                  m_errorsTerminal );

    m_telegramEditorFactory = new Agape::Editor::Factory( *m_largeDialogueTerminal,
                                                          *m_telegramAssetLoaderFactory,
                                                          nullptr,
                                                          *m_timerFactory );

    m_sceneItemTextEditorFactory = new Agape::Editor::Factory( *m_largeDialogueTerminal,
                                                               *m_assetLoaderFactory,
                                                               nullptr,
                                                               *m_timerFactory );

    m_ansiEditorFactory = new Agape::ANSIEditor::Factory( *m_mapTerminal,
                                                          *m_presenceTerminal,
                                                          *m_navigationTerminal,
                                                          *m_assetLoaderFactory,
                                                          *m_worldUser,
                                                          *m_timerFactory,
                                                          *m_clock );

    m_chat = new Chat( *m_coordinates,
                       *m_worldMetadata,
                       *m_worldUser,
                       *m_encryptorFactory,
                       *m_tupleRouter,
                       *m_windowManager,
                       _Chat,
                       Point( 80, -112 ),
                       Point( 80, 0 ),
                    *m_platform,
                    *m_timerFactory );
    
    m_tabBar = new UI::TabBar( *m_windowManager,
                               _Status );

    m_hotkeys = new UI::Hotkeys( *m_windowManager,
                                 _Hotkeys );
    
    m_navigation = new UI::Navigation( *m_windowManager,
                                       _Navigation,
                                       *m_coordinates,
                                       *m_worldMetadata );

    m_presence = new UI::Presence( *m_windowManager,
                                   _Presence,
                                   *m_coordinates,
                                   *m_worldMetadata,
                                   *m_presenceLoaderFactory,
                                   *m_clock );
    
    m_platformUI = new UI::PlatformUI( *m_platform,
                                       *m_windowManager,
                                       _Map,
                                       *m_tabBar,
                                       *m_inputDevice,
                                       *m_timerFactory );

    m_notificationsUI = new UI::NotificationsUI( *m_telegramLoaderFactory,
                                                 *m_coordinates,
                                                 *m_worldUser,
                                                 *m_tabBar,
                                                 *m_platform,
                                                 *m_worldbook );

    buildSplash();

    LOG_DEBUG( "Building UI strategies" );
    m_connectWiFi = new UI::Strategies::ConnectWiFi( *m_inputDevice,
                                                     *m_windowManager,
                                                     _Map,
                                                     *m_line,
                                                     *m_timerFactory,
                                                     *m_dialogue );

    m_connect = new UI::Strategies::Connect( *m_inputDevice,
                                             *m_windowManager,
                                             _Map,
                                             *m_phonebook,
                                             *m_configurationStore,
                                             *m_line,
                                             *m_timerFactory,
                                             *m_dialogue );

    m_onboarding = new UI::Strategies::Onboarding( *m_inputDevice,
                                                   *m_windowManager,
                                                   *m_phonebook,
                                                   *m_worldbook,
                                                   *m_worldMetadata,
                                                   *m_worldUser,
                                                   *m_configurationStore,
                                                   *m_clock,
                                                   *m_snowflake,
                                                   _Map,
                                                   _LargeDialogue,
                                                   _defaultPhoneName,
                                                   _defaultPhoneNumber,
                                                   _learningWorldID,
                                                   (char*)_learningWorldKey );

    m_menu = new UI::Strategies::Menu( *m_inputDevice,
                                       *m_windowManager,
                                       _Map,
                                       chooser,
                                       _Status );

    m_settings = new UI::Strategies::Settings( *m_inputDevice,
                                               *m_windowManager,
                                               _Map );

    m_modemConfig = new UI::Strategies::ModemConfig( *m_inputDevice,
                                                     *m_windowManager,
                                                     _Map,
                                                     *m_line );

    m_phonebookStrategy = new UI::Strategies::Phonebook( *m_windowManager,
                                                         _Map,
                                                         *m_inputDevice,
                                                         *m_phonebook,
                                                         *m_dialogue );
    
    m_worldbookStrategy = new UI::Strategies::Worldbook( *m_windowManager,
                                                         _Map,
                                                         *m_inputDevice,
                                                         *m_worldbook,
                                                         *m_worldUtilities,
                                                         *m_configurationStore,
                                                         *m_line,
                                                         *m_dialogue );

    m_chooseAvatar = new UI::Strategies::ChooseAvatar( *m_inputDevice,
                                                       *m_windowManager,
                                                       _Map,
                                                       *m_entropySource,
                                                       *m_timerFactory );

    m_createWorldStrategy = new UI::Strategies::CreateWorld( *m_inputDevice,
                                                             *m_windowManager,
                                                             _Map,
                                                             *m_worldUtilities,
                                                             *m_worldLoaderFactory,
                                                             *m_timerFactory,
                                                             *m_dialogue );

    m_joinWorldStrategy = new UI::Strategies::JoinWorld( *m_inputDevice,
                                                         *m_windowManager,
                                                         _Map,
                                                         *m_worldbook,
                                                         *m_worldUtilities,
                                                         *m_worldLoaderFactory,
                                                         *m_timerFactory,
                                                         *m_dialogue );

    m_enterWorldStrategy = new UI::Strategies::EnterWorld( *m_inputDevice,
                                                           *m_worldbook,
                                                           *m_line,
                                                           *m_worldMetadata,
                                                           *m_worldUser,
                                                           *m_worldUtilities,
                                                           *m_worldLoaderFactory,
                                                           *m_configurationStore,
                                                           *m_dialogue );

    m_walk = new UI::Strategies::Walk( *m_inputDevice,
                                       *m_compositor,
                                       *m_chat,
                                       *m_worldMetadata,
                                       *m_worldUser,
                                       *m_coordinates,
                                       *m_hotkeys,
                                       *m_navigation,
                                       *m_presence,
                                       *m_vrTime,
                                       *m_platform,
                                       *m_platformUI,
                                       *m_notificationsUI,
                                       *m_timerFactory,
                                       *m_onboarding,
                                       *m_entropySource,
                                       *m_clock,
                                       *m_line,
                                       *m_dialogue,
                                       *m_midiPlayer );

    m_editWorld = new UI::Strategies::EditWorld( *m_inputDevice,
                                                 *m_windowManager,
                                                 _Map,
                                                 _LargeDialogue,
                                                 *m_dialogue,
                                                 *m_hotkeys,
                                                 *m_timerFactory,
                                                 *m_assetLoaderFactory,
                                                 *m_sceneItemTextEditorFactory,
                                                 *m_worldMetadata,
                                                 *m_coordinates,
                                                 *m_compositor );
    
    m_carlo = new UI::Strategies::Carlo( *m_inputDevice,
                                         *m_compositor,
                                         *m_editorFactory,
                                         *m_coordinates,
                                         *m_programManager,
                                         *m_programAssetLoaderFactory,
                                         *m_dialogue,
                                         *m_hotkeys,
                                         *m_windowManager,
                                         _Errors,
                                         _LargeDialogue,
                                         *m_timerFactory );

    m_linda2 = new UI::Strategies::Linda2( *m_inputDevice,
                                           *m_compositor,
                                           *m_windowManager,
                                           *m_hotkeys,
                                           *m_tupleRouter,
                                           *m_functionDispatcher,
                                           _Linda2 );

    m_enterKeyStrategy = new UI::Strategies::EnterKey( *m_windowManager,
                                                       _Map,
                                                       *m_inputDevice,
                                                       *m_keyUtilities,
                                                       *m_configurationStore,
                                                       *m_dialogue,
                                                       *m_timerFactory );
    
    // FIXME: Separate this out to the Tela builder (and have a hook here so
    // that the sub-builder can add to the strategies list, and set the
    // default strategy for Session.
    m_telaStrategy = new UI::Strategies::Tela( *m_inputDevice,
                                               *m_worldLoaderFactory,
                                               *m_configurationStore,
                                               *m_worldUtilities,
                                               *m_keyUtilities,
                                               *m_phonebook,
                                               *m_worldbook,
                                               *m_dialogue,
                                               _defaultPhoneName,
                                               _defaultPhoneNumber );

    m_telegramStrategy = new UI::Strategies::Telegram( *m_inputDevice,
                                                       *m_telegramLoaderFactory,
                                                       *m_worldUser,
                                                       *m_worldMetadata,
                                                       *m_worldbook,
                                                       *m_telegramAssetLoaderFactory,
                                                       *m_telegramEditorFactory,
                                                       *m_encryptorFactory,
                                                       *m_windowManager,
                                                       _LargeDialogue,
                                                       *m_dialogue,
                                                       *m_timerFactory,
                                                       *m_clock,
                                                       *m_snowflake );

    m_teleportStrategy = new UI::Strategies::Teleport( *m_inputDevice,
                                                       *m_worldLoaderFactory,
                                                       *m_presenceLoaderFactory,
                                                       *m_worldUser,
                                                       *m_worldMetadata,
                                                       *m_coordinates,
                                                       *m_worldbook,
                                                       *m_configurationStore,
                                                       *m_encryptorFactory,
                                                       *m_snowflake,
                                                       *m_windowManager,
                                                       _LargeDialogue,
                                                       *m_dialogue,
                                                       *m_timerFactory );

    m_woodStrategy = new UI::Strategies::Wood( *m_inputDevice,
                                               *m_worldLoaderFactory,
                                               *m_worldUtilities,
                                               *m_worldMetadata,
                                               *m_worldUser,
                                               *m_worldbook,
                                               *m_windowManager,
                                               *m_hotkeys,
                                               *m_timerFactory,
                                               _Map,
                                               _Chat,
                                               _Navigation );

    m_inviteFriendStrategy = new UI::Strategies::InviteFriend( *m_inputDevice,
                                                               *m_worldbook,
                                                               *m_worldMetadata,
                                                               *m_keyUtilities,
                                                               *m_tupleRouter,
                                                               *m_windowManager,
                                                               _LargeDialogue,
                                                               *m_dialogue,
                                                               *m_timerFactory );

    m_miniMap = new MiniMap( *m_sceneLoaderFactory,
                             *m_assetLoaderFactory,
                             *m_drawTerminal );

    buildMiniMapAssetLoaderFactory();

    m_miniMapStrategy = new UI::Strategies::MiniMap( *m_miniMapAssetLoaderFactory,
                                                     *m_coordinates,
                                                     *m_navigation,
                                                     *m_inputDevice,
                                                     *m_hotkeys,
                                                     *m_windowManager,
                                                     _Map,
                                                     *m_dialogue );

    m_creditsStrategy = new UI::Strategies::Credits( *m_inputDevice,
                                                     *m_windowManager,
                                                     _Map,
                                                     *m_timerFactory );

    m_ansiEditorStrategy = new UI::Strategies::ANSIEditor( *m_inputDevice,
                                                           *m_compositor,
                                                           *m_ansiEditorFactory,
                                                           *m_assetLoaderFactory,
                                                           *m_programAssetLoaderFactory,
                                                           *m_coordinates,
                                                           *m_dialogue,
                                                           *m_hotkeys,
                                                           *m_windowManager,
                                                           _LargeDialogue,
                                                           *m_timerFactory );

    m_messageStrategy = new UI::Strategies::Message( *m_windowManager,
                                                     _LargeDialogue,
                                                     *m_inputDevice,
                                                     *m_timerFactory,
                                                     *m_assetLoaderFactory,
                                                     *m_coordinates,
                                                     *m_configurationStore );

    m_worldActor = new Agape::Linda2::Actors::NativeActors::World( *m_tupleRouter,
                                                                   *m_functionDispatcher,
                                                                   *m_compositor,
                                                                   *m_snowflake,
                                                                   *m_coordinates,
                                                                   *m_worldbook );

    m_musician = new Agape::Linda2::Actors::NativeActors::Musician( *m_tupleRouter, *m_midiPlayer );

    m_snowflakeActor = new Agape::Linda2::Actors::NativeActors::Snowflake( *m_functionDispatcher );

    m_thingActor = new Agape::Linda2::Actors::NativeActors::Thing( *m_tupleRouter,
                                                                   *m_functionDispatcher,
                                                                   *m_compositor );
    
    m_userActor = new Agape::Linda2::Actors::NativeActors::User( *m_tupleRouter,
                                                                 *m_functionDispatcher,
                                                                 *m_compositor,
                                                                 *m_worldUser );

    m_platformActor = new Agape::Linda2::Actors::NativeActors::Platform( *m_functionDispatcher,
                                                                         *m_platform );

    m_entropyActor = new Agape::Linda2::Actors::NativeActors::Entropy( *m_functionDispatcher,
                                                                       *m_entropySource );

    m_eventTimerActor = new Agape::Linda2::Actors::NativeActors::EventTimer( *m_timerFactory,
                                                                             *m_tupleRouter );

    LOG_DEBUG( "Building session" );
    Map< String, UI::Strategy* > strategies;
    strategies[_splash] = m_splash;
    strategies[_connectWiFi] = m_connectWiFi;
    strategies[_connect] = m_connect;
    strategies[_onboarding] = m_onboarding;
    strategies[_menu] = m_menu;
    strategies[_settings] = m_settings;
    strategies[_modemConfig] = m_modemConfig;
    strategies[_phonebook] = m_phonebookStrategy;
    strategies[_worldbook] = m_worldbookStrategy;
    strategies[_chooseAvatar] = m_chooseAvatar;
    strategies[_createWorld] = m_createWorldStrategy;
    strategies[_joinWorld] = m_joinWorldStrategy;
    strategies[_enterWorld] = m_enterWorldStrategy;
    strategies[_walk] = m_walk;
    strategies[_edit] = m_editWorld;
    strategies[_carlo] = m_carlo;
    strategies[_linda2] = m_linda2;
    strategies[_enterKey] = m_enterKeyStrategy;
    strategies[_tela] = m_telaStrategy;
    strategies[_minimap] = m_miniMapStrategy;
    strategies[_telegram] = m_telegramStrategy;
    strategies[_teleport] = m_teleportStrategy;
    strategies[_wood] = m_woodStrategy;
    strategies[_inviteFriend] = m_inviteFriendStrategy;
    strategies[_credits] = m_creditsStrategy;
    strategies[_ansiEditor] = m_ansiEditorStrategy;
    strategies[_message] = m_messageStrategy;

    if( m_memoryStrategy )
    {
        strategies[_memory] = m_memoryStrategy;
    }

    if( m_testStrategy )
    {
        strategies[_test] = m_testStrategy;
    }

    m_session = new Session( strategies, _splash );

    m_connectionMonitor = new ConnectionMonitor( *m_line,
                                                 *m_configurationStore,
                                                 *m_tupleRouter,
                                                 *m_tupleRoute,
                                                 *m_tabBar,
                                                 *m_vrTime );

    LOG_DEBUG( "Building client" );
    m_client = new Client( *m_line,
                           *m_connectionMonitor,
                           *m_platform,
                           *m_platformUI,
                           *m_inputDevice,
                           *m_entropySource,
                           *m_midiPlayer,
                           *m_eventClock,
                           *m_eventTimerActor,
                           *m_session );

    return m_client;
}

void ClientBuilder::unbuild()
{
    LOG_DEBUG( "ClientBuilder: Unbuilding" );
    _unbuild();
    deleteMembers();
    setMembersNull();
}

void ClientBuilder::deleteMembers()
{
    LOG_DEBUG( "ClientBuilder: Deleting members" );
    delete( m_client );
    delete( m_connectionMonitor );
    delete( m_session );
    delete( m_eventTimerActor );
    delete( m_entropyActor );
    delete( m_platformActor );
    delete( m_userActor );
    delete( m_thingActor );
    delete( m_snowflakeActor );
    delete( m_musician );
    delete( m_worldActor );
    delete( m_messageStrategy );
    delete( m_ansiEditorStrategy );
    delete( m_creditsStrategy );
    delete( m_miniMapStrategy );
    delete( m_miniMapAssetLoaderFactory );
    delete( m_miniMap );
    delete( m_inviteFriendStrategy );
    delete( m_woodStrategy );
    delete( m_teleportStrategy );
    delete( m_telegramStrategy );
    delete( m_telaStrategy );
    delete( m_enterKeyStrategy );
    delete( m_linda2 );
    delete( m_carlo );
    delete( m_editWorld );
    delete( m_walk );
    delete( m_enterWorldStrategy );
    delete( m_createWorldStrategy );
    delete( m_joinWorldStrategy );
    delete( m_chooseAvatar );
    delete( m_worldbookStrategy );
    delete( m_phonebookStrategy );
    delete( m_modemConfig );
    delete( m_settings );
    delete( m_menu );
    delete( m_onboarding );
    delete( m_connect );
    delete( m_connectWiFi );
    delete( m_testStrategy );
    delete( m_memoryStrategy );
    delete( m_splash );
    delete( m_notificationsUI );
    delete( m_platformUI );
    delete( m_presence );
    delete( m_navigation );
    delete( m_hotkeys );
    delete( m_tabBar );
    delete( m_chat );
    delete( m_ansiEditorFactory );
    delete( m_sceneItemTextEditorFactory );
    delete( m_telegramEditorFactory );
    delete( m_editorFactory );
    delete( m_highlighterFactory );
    delete( m_worldLoaderFactory );
    delete( m_worldUtilities );
    delete( m_keyUtilities );
    delete( m_compositor );
    delete( m_worldbook );
    delete( m_phonebook );
    delete( m_midiPlayer );
    delete( m_telegramLoaderFactory );
    delete( m_presenceLoaderFactory );
    delete( m_sceneLoaderFactory );
    delete( m_midiAssetLoaderFactory );
    delete( m_telegramAssetLoaderFactory );
    delete( m_programAssetLoaderFactory );
    delete( m_assetLoaderFactory );
    delete( m_worldUser );
    delete( m_worldMetadata );
    delete( m_coordinates );
    delete( m_hash );
    delete( m_blockEncryptorFactory );
    delete( m_encryptorFactory );
    delete( m_entropySource );
    delete( m_programManager );
    delete( m_snowflake );
    delete( m_eventClock );
    delete( m_clock );
    delete( m_vrTime );
    delete( m_functionDispatcher );
    delete( m_tupleRouter );
    delete( m_tupleRoute );
    delete( m_tupleDispatcher );
    delete( m_inputDevice );
    delete( m_line );
    delete( m_lineDriver );
    delete( m_platform );
    delete( m_rwBuffer );
    delete( m_dialogue );
    delete( m_linda2Terminal );
    delete( m_errorsTerminal );
    delete( m_smallDialogueTerminal );
    delete( m_largeDialogueTerminal );
    delete( m_chatTerminal );
    delete( m_presenceTerminal );
    delete( m_hotkeysTerminal );
    delete( m_statusTerminal );
    delete( m_navigationTerminal );
    delete( m_mapTerminal );
    delete( m_drawTerminal );
    delete( m_nullTimer );
    delete( m_headlessGraphicsDriver );
    delete( m_animationTimer );
    delete( m_windowManager );
    delete( m_graphicsDriver );
    delete( m_configurationStore );
    delete( m_performanceTimerFactory );
    delete( m_timerFactory );
}

void ClientBuilder::setMembersNull()
{
    LOG_DEBUG( "ClientBuilder: Setting members to nullptr" );
    m_timerFactory = nullptr;
    m_performanceTimerFactory = nullptr;
    m_configurationStore = nullptr;
    m_graphicsDriver = nullptr;
    m_windowManager = nullptr;
    m_dialogue = nullptr;
    m_rwBuffer = nullptr;
    m_platform = nullptr;
    m_lineDriver = nullptr;
    m_line = nullptr;
    m_inputDevice = nullptr;
    m_tupleDispatcher = nullptr;
    m_tupleRoute = nullptr;
    m_tupleRouter = nullptr;
    m_functionDispatcher = nullptr;
    m_vrTime = nullptr;
    m_clock = nullptr;
    m_eventClock = nullptr;
    m_snowflake = nullptr;
    m_programManager = nullptr;
    m_entropySource = nullptr;
    m_encryptorFactory = nullptr;
    m_blockEncryptorFactory = nullptr;
    m_hash = nullptr;
    m_worldMetadata = nullptr;
    m_worldUser = nullptr;
    m_assetLoaderFactory = nullptr;
    m_programAssetLoaderFactory = nullptr;
    m_telegramAssetLoaderFactory = nullptr;
    m_midiAssetLoaderFactory = nullptr;
    m_sceneLoaderFactory = nullptr;
    m_presenceLoaderFactory = nullptr;
    m_telegramLoaderFactory = nullptr;
    m_midiPlayer = nullptr;
    m_worldUtilities = nullptr;
    m_worldLoaderFactory = nullptr;
    m_splash = nullptr;
    m_memoryStrategy = nullptr;
    m_testStrategy = nullptr;
    m_animationTimer = nullptr;
    m_headlessGraphicsDriver = nullptr;
    m_nullTimer = nullptr;
    m_drawTerminal = nullptr;
    m_mapTerminal = nullptr;
    m_navigationTerminal = nullptr;
    m_statusTerminal = nullptr;
    m_hotkeysTerminal = nullptr;
    m_presenceTerminal = nullptr;
    m_chatTerminal = nullptr;
    m_largeDialogueTerminal = nullptr;
    m_smallDialogueTerminal = nullptr;
    m_errorsTerminal = nullptr;
    m_linda2Terminal = nullptr;
    m_coordinates = nullptr;
    m_phonebook = nullptr;
    m_worldbook = nullptr;
    m_compositor = nullptr;
    m_keyUtilities = nullptr;
    m_highlighterFactory = nullptr;
    m_editorFactory = nullptr;
    m_telegramEditorFactory = nullptr;
    m_sceneItemTextEditorFactory = nullptr;
    m_ansiEditorFactory = nullptr;
    m_chat = nullptr;
    m_tabBar = nullptr;
    m_hotkeys = nullptr;
    m_navigation = nullptr;
    m_presence = nullptr;
    m_platformUI = nullptr;
    m_notificationsUI = nullptr;
    m_connectWiFi = nullptr;
    m_connect = nullptr;
    m_onboarding = nullptr;
    m_menu = nullptr;
    m_settings = nullptr;
    m_modemConfig = nullptr;
    m_phonebookStrategy = nullptr;
    m_worldbookStrategy = nullptr;
    m_chooseAvatar = nullptr;
    m_createWorldStrategy = nullptr;
    m_joinWorldStrategy = nullptr;
    m_enterWorldStrategy = nullptr;
    m_walk = nullptr;
    m_editWorld = nullptr;
    m_carlo = nullptr;
    m_linda2 = nullptr;
    m_enterKeyStrategy = nullptr;
    m_telaStrategy = nullptr;
    m_telegramStrategy = nullptr;
    m_teleportStrategy = nullptr;
    m_woodStrategy = nullptr;
    m_inviteFriendStrategy = nullptr;
    m_miniMap = nullptr;
    m_miniMapAssetLoaderFactory = nullptr;
    m_miniMapStrategy = nullptr;
    m_creditsStrategy = nullptr;
    m_ansiEditorStrategy = nullptr;
    m_messageStrategy = nullptr;
    m_worldActor = nullptr;
    m_musician = nullptr;
    m_snowflakeActor = nullptr;
    m_thingActor = nullptr;
    m_userActor = nullptr;
    m_platformActor = nullptr;
    m_entropyActor = nullptr;
    m_eventTimerActor = nullptr;
    m_session = nullptr;
    m_connectionMonitor = nullptr;
    m_client = nullptr;
}

} // namespace Agape
