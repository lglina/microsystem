#include "AssetLoaders/Caches/KiamaFSAssetCache.h"
#include "AssetLoaders/Factories/CacheAssetLoaderFactory.h"
#include "AssetLoaders/Factories/KiamaFSAssetLoaderFactory.h"
#include "AssetLoaders/Factories/MiniMapAssetLoaderFactory.h"
#include "Audio/MIDIPlayers/SAM2695MIDIPlayer.h"
#include "Clocks/VRTimeClock.h"
#include "EntropySources/PIC32EntropySource.h"
#include "EntropySources/SPIEntropySource.h"
#include "EventClocks/NullEventClock.h"
#include "GraphicsDrivers/RA8873.h"
#include "InputDevices/SPIKeyboard.h"
#include "LineDrivers/NullLineDriver.h"
#include "Lines/DummyModemLine.h"
#include "Loggers/Logger.h"
#include "Memories/KiamaFSMemory.h"
#include "Memories/SPIFlash.h"
#include "Platforms/BoopiePlatform.h"
#include "PresenceLoaders/Factories/OfflinePresenceLoaderFactory.h"
#include "PresenceLoaders/OfflinePresenceStore.h"
#include "SceneLoaders/Factories/KiamaFSSceneLoaderFactory.h"
#include "SceneLoaders/Factories/Linda2SceneLoaderFactory.h"
#include "SceneLoaders/Linda2SceneLoaderResponder.h"
#include "TelegramLoaders/Factories/NullTelegramLoaderFactory.h"
#include "Timers/Factories/PIC32PrecisionTimerFactory.h"
#include "Timers/Factories/PIC32TimerFactory.h"
#include "Timers/PIC32AbsoluteTimer.h"
#include "TupleRoutes/NullTupleRoute.h"
#include "UI/Strategies/MemoryStrategy.h"
#include "UI/Strategies/Splash.h"
#include "UI/Strategies/TestStrategy.h"
#include "UI/Strategies/UpdateStrategy.h"
#include "UI/VRTime.h"
#include "WorldLoaders/Factories/KiamaFSWorldLoaderFactory.h"
#include "BoopieOfflineClientBuilder.h"
#include "BusController.h"
#include "InterruptHandler.h"
#include "KiamaFS.h"
#include "PICSerial.h"
#include "SPIController.h"
#include "SPIRequester.h"
#include "String.h"

#include <xc.h>

namespace Agape
{

namespace ClientBuilders
{

BoopieOffline::BoopieOffline( ReadableWritable& debugSerial ) :
  m_debugSerial( debugSerial )
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
    delete( m_miniMapAssetCache );
    delete( m_miniMapAssetLoaderBackingFactory );
    delete( m_midiSerial );
    delete( m_offlinePresenceStore );
    delete( m_sceneLoaderResponder );
    delete( m_sceneLoaderBackingFactory );
    delete( m_spiRequester );
    delete( m_spiController );
    delete( m_configurationStoreMemory );
    delete( m_fs );
    delete( m_fsMemory );
    delete( m_flash );
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
    m_flash = nullptr;
    m_fsMemory = nullptr;
    m_fs = nullptr;
    m_configurationStoreMemory = nullptr;
    m_spiController = nullptr;
    m_spiRequester = nullptr;
    m_sceneLoaderBackingFactory = nullptr;
    m_sceneLoaderResponder = nullptr;
    m_offlinePresenceStore = nullptr;
    m_midiSerial = nullptr;
    m_miniMapAssetLoaderBackingFactory = nullptr;
    m_miniMapAssetCache = nullptr;
}

void BoopieOffline::getMachineID()
{
    m_machineID = DEVSN0;;
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
    m_spiController = new SPIController( 1,
                                         4000000, // Hz
                                         true, // true = master
                                         *m_timerFactory );
    m_spiRequester = new SPIRequester( *m_spiController, *m_bus );
    m_flash = new Memories::SPIFlash( *m_spiController,
                                      *m_bus,
                                      0x800000,
                                      0x100,
                                      0x1000,
                                      0x10000,
                                      0x0 );
    m_fsMemory = new Memories::SPIFlash( *m_spiController,
                                         *m_bus,
                                         0x100000,
                                         0x100,
                                         0x1000,
                                         0x10000,
                                         0x0 );
    LOG_DEBUG( "Initialising filesystem" );
    m_fs = new KiamaFS( *m_fsMemory );
    LOG_DEBUG( "Done." );
    m_configurationStoreMemory = new Memories::KiamaFS( *m_fs, "config-offline.dat" );

    m_configurationStore = new ConfigurationStore( *m_configurationStoreMemory );
}

void BoopieOffline::buildGraphicsDriver()
{
    m_graphicsDriver = new GraphicsDrivers::RA8873( *m_bus, *m_pic32PrecisionTimerFactory );
    InterruptDispatcher::s_graphicsDriver = m_graphicsDriver;
}

void BoopieOffline::buildPlatform()
{
    m_platform = new Platforms::Boopie( *m_graphicsDriver, *m_bus, *m_spiRequester, *m_timerFactory, m_fs );
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

    m_memoryStrategy = new UI::Strategies::Memory( *m_windowManager,
                                                   _Map,
                                                   *m_inputDevice,
                                                   *m_flash,
                                                   m_debugSerial,
                                                   *m_pic32PrecisionTimerFactory );
}

void BoopieOffline::buildTupleRoute()
{
    m_tupleRoute = new TupleRoutes::Null( "Null" );
}

void BoopieOffline::buildClock()
{
    m_clock = new Clocks::VRTime( *m_vrTime );
    // FIXME: Should be real to get offline ticks, but we have no RTC!
    m_eventClock = new EventClocks::Null( *m_tupleRouter );
}

void BoopieOffline::buildEntropySource()
{
    m_entropySource = new EntropySources::PIC32TRNG;
    m_keyEntropySource = new EntropySources::SPI( *m_spiRequester, *m_timerFactory );
}

void BoopieOffline::buildAssetLoaderFactory()
{
    m_assetLoaderFactory = new AssetLoaders::Factories::KiamaFS( *m_fs, "ans" );
}

void BoopieOffline::buildProgramAssetLoaderFactory()
{
    m_assetLoaderFactory = new AssetLoaders::Factories::KiamaFS( *m_fs, "cl2" );
}

void BoopieOffline::buildTelegramAssetLoaderFactory()
{
    m_telegramAssetLoaderFactory = new AssetLoaders::Factories::KiamaFS( *m_fs, "tgm" );
}

void BoopieOffline::buildMIDIAssetLoaderFactory()
{
    m_midiAssetLoaderFactory = new AssetLoaders::Factories::Baked();
}

void BoopieOffline::buildSceneLoaderFactory()
{
    m_sceneLoaderFactory = new SceneLoaders::Factories::Linda2( *m_tupleRouter, *m_timerFactory );
    m_sceneLoaderBackingFactory = new SceneLoaders::Factories::KiamaFS( *m_fs );
    m_sceneLoaderResponder = new SceneLoaders::Linda2Responder( *m_tupleRouter, *m_sceneLoaderBackingFactory );
}

void BoopieOffline::buildPresenceLoaderFactory()
{
    m_presenceLoaderFactory = new PresenceLoaders::Factories::Offline( *m_offlinePresenceStore, *m_clock );
}

void BoopieOffline::buildTelegramLoaderFactory()
{
    m_telegramLoaderFactory = new TelegramLoaders::Factories::Null();
}

void BoopieOffline::buildMIDIPlayer()
{
    m_midiSerial = new PICSerial( 3, 31250, 128, 16 );
    m_midiPlayer = new Audio::MIDIPlayers::SAM2695( *m_midiAssetLoaderFactory, *m_midiSerial );
}

void BoopieOffline::buildWorldLoaderFactory()
{
    m_worldLoaderFactory = new WorldLoaders::Factories::KiamaFS( *m_fs );
}

void BoopieOffline::buildSplash()
{
    m_splash = new UI::Strategies::Splash( *m_windowManager,
                                           *m_inputDevice,
                                           *m_timerFactory,
                                           *m_platform );

    m_testStrategy = new UI::Strategies::Test( *m_windowManager,
                                               _Map,
                                               *m_inputDevice,
                                               *m_platform,
                                               *m_flash,
                                               *m_midiPlayer,
                                               *m_entropySource,
                                               *m_timerFactory );
}

void BoopieOffline::buildMiniMapAssetLoaderFactory()
{
    m_miniMapAssetLoaderBackingFactory = new AssetLoaders::Factories::MiniMap( *m_miniMap );
    m_miniMapAssetCache = new AssetLoaders::Caches::KiamaFSAssetCache( 50,
                                                                       512,
                                                                       *m_fs,
                                                                       "map",
                                                                       *m_hash,
                                                                       *m_clock );
    m_miniMapAssetLoaderFactory = new AssetLoaders::Factories::Cache( *m_miniMapAssetLoaderBackingFactory,
                                                                      *m_miniMapAssetCache,
                                                                      false );
}

void BoopieOffline::buildUpdate()
{
    m_updateMemory = new Memories::SPIFlash( *m_spiController,
                                             *m_bus,
                                             0x200000,
                                             0x100,
                                             0x1000,
                                             0x10000,
                                             0x600000 );
    m_updateStrategy = new Strategies::Update( *m_inputDevice,
                                               *m_windowManager,
                                               _LargeDialogue,
                                               *m_tupleRouter,
                                               *m_platform,
                                               *m_updateMemory,
                                               *m_dialogue,
                                               *m_timerFactory );
}

} // namespace ClientBuilders

} // namespace Agape
