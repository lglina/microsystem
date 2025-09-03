#include "AssetLoaders/Factories/DemoAssetLoaderFactory.h"
#include "AssetLoaders/Factories/MiniMapAssetLoaderFactory.h"
#include "AssetLoaders/Factories/RAMAssetLoaderFactory.h"
#include "Audio/MIDIPlayers/NullMIDIPlayer.h"
#include "Clocks/DummyClock.h"
#include "EntropySources/DummyEntropySource.h"
#include "EventClocks/DummyEventClock.h"
#include "GraphicsDrivers/RA8873.h"
#include "InputDevices/SPIKeyboard.h"
#include "LineDrivers/NullLineDriver.h"
#include "Lines/DummyModemLine.h"
#include "Memories/RAMMemory.h"
#include "Platforms/BoopiePlatform.h"
#include "PresenceLoaders/Factories/DummyPresenceLoaderFactory.h"
#include "SceneLoaders/Factories/DemoSceneLoaderFactory.h"
#include "SceneLoaders/Factories/Linda2SceneLoaderFactory.h"
#include "SceneLoaders/Linda2SceneLoaderResponder.h"
#include "TelegramLoaders/Factories/DummyTelegramLoaderFactory.h"
#include "Timers/Factories/PIC32PrecisionTimerFactory.h"
#include "Timers/Factories/PIC32TimerFactory.h"
#include "Timers/PIC32AbsoluteTimer.h"
#include "TupleRoutes/NullTupleRoute.h"
#include "UI/Strategies/Splash.h"
#include "WorldLoaders/Factories/DummyWorldLoaderFactory.h"
#include "BoopieOfflineClientBuilder.h"
#include "BusController.h"
#include "DummyData.h"
#include "InterruptHandler.h"
#include "KiamaFS.h"
#include "PICSerial.h"
#include "SPIController.h"
#include "SPIRequester.h"
#include "String.h"
#include "Warp.h"

namespace Agape
{

namespace ClientBuilders
{

BoopieOffline::BoopieOffline()
{
    _setMembersNull();
}

BoopieOffline::~BoopieOffline()
{
    _deleteMembers();
}

void BoopieOffline::_unbuild()
{
    _deleteMembers();
    _setMembersNull();
}

void BoopieOffline::_deleteMembers()
{
    delete( m_sceneLoaderResponder );
    delete( m_sceneLoaderBackingFactory );
    delete( m_spiRequester );
    delete( m_spiController );
    delete( m_configurationStoreMemory );
    //delete( m_fs );
    //delete( m_flash );
    delete( m_pic32PrecisionTimerFactory );
    delete( m_absoluteTimer );
    delete( m_bus );
    InterruptDispatcher::s_graphicsDriver = nullptr;
}

void BoopieOffline::_setMembersNull()
{
    m_bus = nullptr;
    m_absoluteTimer = nullptr;
    m_pic32PrecisionTimerFactory = nullptr;
    //m_flash = nullptr;
    //m_fs = nullptr;
    m_configurationStoreMemory = nullptr;
    m_spiController = nullptr;
    m_spiRequester = nullptr;
    m_sceneLoaderBackingFactory = nullptr;
    m_sceneLoaderResponder = nullptr;
}

void BoopieOffline::getMachineID()
{
    m_machineID = 0;
}

void BoopieOffline::buildTimerFactory()
{
    m_bus = new BusController;
    m_absoluteTimer = new Timers::PIC32Absolute( m_bus );
    m_timerFactory = new Timers::Factories::PIC32TimerFactory;
    m_pic32PrecisionTimerFactory = new Timers::Factories::PIC32PrecisionTimerFactory;
}

void BoopieOffline::buildPerformanceTimerFactory()
{
    m_performanceTimerFactory = new Timers::Factories::PIC32TimerFactory;
}

void BoopieOffline::buildConfigurationStore()
{
    Memories::RAM* ramMemory( new Memories::RAM( 2048, 256, Memory::eeprom ) );
    World::Coordinates coordinates;
    AssetLoaders::Factories::Demo demoLoaderFactory;
    AssetLoader* assetLoader( demoLoaderFactory.makeLoader( coordinates, "config-memory" ) );
    ramMemory->loadFromAsset( *assetLoader );
    delete( assetLoader );
    m_configurationStoreMemory = ramMemory;
    /*
    m_spiController = new SPIController( 2, true ); // FIXME: Switch to SPI1 for BetaBoard?
    m_bus = new BusController;
    m_flash = new Memories::SPIFlash( *m_spiController, *m_bus );
    m_fs = new KiamaFS( *m_flash );
    //m_configurationStoreMemory = new Memories::RAM( 1024, -1, Memory::eeprom );
    m_configurationStoreMemory = new Memories::KiamaFS( *m_fs, "config.dat" );
    */
    m_configurationStore = new ConfigurationStore( *m_configurationStoreMemory );
}

void BoopieOffline::buildGraphicsDriver()
{
    m_graphicsDriver = new GraphicsDrivers::RA8873( *m_bus, *m_pic32PrecisionTimerFactory );
    InterruptDispatcher::s_graphicsDriver = m_graphicsDriver;
}

void BoopieOffline::buildPlatform()
{
    m_spiController = new SPIController( 1,
                                         4000000, // Hz
                                         true, // true = master
                                         *m_timerFactory );
#if defined(__PIC32MX__)
    InterruptDispatcher::instance()->registerHandler( InterruptDispatcher::SPI1, m_spiController );
#elif defined(__PIC32MZ__)
    InterruptDispatcher::instance()->registerHandler( InterruptDispatcher::SPI1Tx, m_spiController );
    InterruptDispatcher::instance()->registerHandler( InterruptDispatcher::SPI1Rx, m_spiController );
#endif
    m_spiRequester = new SPIRequester( *m_spiController, *m_bus );
    m_platform = new Platforms::Boopie( *m_graphicsDriver, *m_bus, *m_spiRequester, *m_timerFactory );
}

void BoopieOffline::buildLineDriver()
{
    m_lineDriver = new LineDrivers::Null;
}

void BoopieOffline::buildLine()
{
    m_line = new Lines::DummyModem( *m_lineDriver, *m_configurationStore, *m_timerFactory );
}

void BoopieOffline::buildInputDevice()
{
    m_inputDevice = new InputDevices::SPIKeyboard( *m_spiRequester, *m_timerFactory );
}

void BoopieOffline::buildTupleRoute()
{
    m_tupleRoute = new TupleRoutes::Null( "Null" );
}

void BoopieOffline::buildClock()
{
    m_clock = new Clocks::Dummy();
    // FIXME: Should be real to get offline ticks, but we have no RTC!
    m_eventClock = new EventClocks::Dummy( *m_tupleRouter );
}

void BoopieOffline::buildEntropySource()
{
    m_entropySource = new EntropySources::Dummy;
}

void BoopieOffline::buildAssetLoaderFactory()
{
    m_assetLoaderFactory = new AssetLoaders::Factories::Demo;
}

void BoopieOffline::buildProgramAssetLoaderFactory()
{
    m_programAssetLoaderFactory = new AssetLoaders::Factories::RAM( *m_assetLoaderFactory );
}

void BoopieOffline::buildTelegramAssetLoaderFactory()
{
    m_telegramAssetLoaderFactory = new AssetLoaders::Factories::Demo;
}

void BoopieOffline::buildMIDIAssetLoaderFactory()
{
    m_midiAssetLoaderFactory = new AssetLoaders::Factories::Demo();
}

void BoopieOffline::buildSceneLoaderFactory()
{
    //m_sceneLoaderFactory = new SceneLoaders::Factories::Remote( *m_sceneLoaderPage );
    //m_sceneLoaderFactory = new SceneLoaders::Factories::Demo( *m_snowflake );
    //m_sceneLoaderFactory = new SceneLoaders::Factories::Linda2( *m_tupleRouter );
    //m_sceneLoaderFactory = new SceneLoaders::Factories::KiamaFS( *m_fs );
    
    //m_sceneLoaderResponderFactory = new SceneLoaders::Factories::Demo;
    //m_sceneLoaderResponder = new SceneLoaders::Linda2Responder( *m_tupleRouter, *m_sceneLoaderResponderFactory );

    m_sceneLoaderFactory = new SceneLoaders::Factories::Linda2( *m_tupleRouter, *m_timerFactory );
    m_sceneLoaderBackingFactory = new SceneLoaders::Factories::Demo( *m_snowflake );
    m_sceneLoaderResponder = new SceneLoaders::Linda2Responder( *m_tupleRouter, *m_sceneLoaderBackingFactory );
}

void BoopieOffline::buildPresenceLoaderFactory()
{
    //m_presenceLoaderFactory = new PresenceLoaders::Factories::Remote( *m_presenceLoaderPage );
    //m_presenceLoaderFactory = new PresenceLoaders::Factories::Demo( *m_windowManager );
    //m_presenceLoaderFactory = new PresenceLoaders::Factories::Linda2( *m_tupleRouter );
    m_presenceLoaderFactory = new PresenceLoaders::Factories::Dummy();
    
    //m_presenceLoaderResponderFactory = new PresenceLoaders::Factories::Demo( *m_windowManager );
    //m_presenceLoaderResponder = new PresenceLoaders::Linda2Responder( *m_tupleRouter, *m_presenceLoaderResponderFactory );
}

void BoopieOffline::buildTelegramLoaderFactory()
{
    m_telegramLoaderFactory = new TelegramLoaders::Factories::Dummy;
}

void BoopieOffline::buildMIDIPlayer()
{
    //m_midiPlayer = new Audio::MIDIPlayers::Null;
    //m_midiSerial = new PICSerial( 2, 31250, 128, 16 );
    //Agape::InterruptDispatcher::instance()->registerHandler( Agape::InterruptDispatcher::UART2, m_midiSerial );
    //m_midiPlayer = new Audio::MIDIPlayers::SAM2695( *m_assetLoaderFactory, *m_midiSerial );
    m_midiPlayer = new Audio::MIDIPlayers::Null();
}

void BoopieOffline::buildWorldLoaderFactory()
{
    m_worldLoaderFactory = new WorldLoaders::Factories::Dummy();
    //m_worldLoaderFactory = new WorldLoaders::Factories::KiamaFS( *m_fs );

    DummyData::generate( *m_entropySource, *m_worldUtilities, *m_clock );
}

void BoopieOffline::buildSplash()
{
    m_splash = new UI::Strategies::Splash( *m_windowManager,
                                           *m_inputDevice,
                                           *m_timerFactory,
                                           *m_platform );
}

void BoopieOffline::buildMiniMapAssetLoaderFactory()
{
    m_miniMapAssetLoaderFactory = new AssetLoaders::Factories::MiniMap( *m_miniMap );
}

} // namespace ClientBuilders

} // namespace Agape
