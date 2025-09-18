#include "AssetLoaders/Caches/KiamaFSAssetCache.h"
#include "AssetLoaders/Factories/CacheAssetLoaderFactory.h"
#include "AssetLoaders/Factories/EncryptedAssetLoaderFactory.h"
#include "AssetLoaders/Factories/Linda2AssetLoaderFactory.h"
#include "AssetLoaders/Factories/MiniMapAssetLoaderFactory.h"
#include "Audio/MIDIPlayers/SAM2695MIDIPlayer.h"
#include "Clocks/VRTimeClock.h"
#include "EntropySources/SPIEntropySource.h"
#include "EventClocks/NullEventClock.h"
#include "GraphicsDrivers/RA8873.h"
#include "InputDevices/SPIKeyboard.h"
#include "LineDrivers/PICSerialLineDriver.h"
#include "Lines/ModemLine.h"
#include "Loggers/Logger.h"
#include "Memories/KiamaFSMemory.h"
#include "Memories/SPIFlash.h"
#include "Platforms/BoopiePlatform.h"
#include "PresenceLoaders/Factories/EncryptedPresenceLoaderFactory.h"
#include "PresenceLoaders/Factories/Linda2PresenceLoaderFactory.h"
#include "SceneLoaders/Factories/EncryptedSceneLoaderFactory.h"
#include "SceneLoaders/Factories/Linda2SceneLoaderFactory.h"
#include "TelegramLoaders/Factories/Linda2TelegramLoaderFactory.h"
#include "Timers/Factories/PIC32PrecisionTimerFactory.h"
#include "Timers/Factories/PIC32TimerFactory.h"
#include "Timers/PIC32AbsoluteTimer.h"
#include "TupleRoutes/ReadableWritableTupleRoute.h"
#include "UI/Strategies/MemoryStrategy.h"
#include "UI/Strategies/Splash.h"
#include "UI/Strategies/TestStrategy.h"
#include "WorldLoaders/Factories/Linda2WorldLoaderFactory.h"
#include "BoopieOnlineClientBuilder.h"
#include "BusController.h"
#include "KiamaFS.h"
#include "PICSerial.h"
#include "ReadableWritable.h"
#include "RWBuffer.h"
#include "SPIController.h"
#include "SPIRequester.h"
#include "String.h"

#include <xc.h>

namespace Agape
{

namespace ClientBuilders
{

BoopieOnline::BoopieOnline( ReadableWritable& debugSerial ) :
  m_debugSerial( debugSerial )
{
    _setMembersNull();
}

BoopieOnline::~BoopieOnline()
{
    _deleteMembers();
}

void BoopieOnline::_unbuild()
{
    _deleteMembers();
    _setMembersNull();
}

void BoopieOnline::_deleteMembers()
{
    delete( m_miniMapAssetCache );
    delete( m_miniMapAssetLoaderBackingFactory );
#if defined(__PIC32MX__)
    Agape::InterruptDispatcher::instance()->deregisterHandler( Agape::InterruptDispatcher::UART3 );
#elif defined(__PIC32MZ__)
    Agape::InterruptDispatcher::instance()->deregisterHandler( Agape::InterruptDispatcher::UART3Tx );
    Agape::InterruptDispatcher::instance()->deregisterHandler( Agape::InterruptDispatcher::UART3Rx );
#endif
    delete( m_midiSerial );
    delete( m_presenceLoaderBackingFactory );
    delete( m_sceneLoaderBackingFactory );
    delete( m_telegramAssetLoaderBackingFactory );
    delete( m_programAssetCache );
    delete( m_programAssetLoaderBackingFactory );
    delete( m_programAssetLoaderEncryptedFactory );
    delete( m_assetCache );
    delete( m_assetLoaderBackingFactory );
    delete( m_assetLoaderEncryptedFactory );
#if defined(__PIC32MX__)
    Agape::InterruptDispatcher::instance()->deregisterHandler( Agape::InterruptDispatcher::UART1 );
#elif defined(__PIC32MZ__)
    Agape::InterruptDispatcher::instance()->deregisterHandler( Agape::InterruptDispatcher::UART1Tx );
    Agape::InterruptDispatcher::instance()->deregisterHandler( Agape::InterruptDispatcher::UART1Rx );
#endif
    delete( m_lineSerial );
    delete( m_configurationStoreMemory );
    delete( m_fs );
    delete( m_flash );
    delete( m_spiRequester );
#if defined(__PIC32MX__)
    InterruptDispatcher::instance()->deregisterHandler( InterruptDispatcher::SPI1 );
#elif defined(__PIC32MZ__)
    InterruptDispatcher::instance()->deregisterHandler( InterruptDispatcher::SPI1Tx );
    InterruptDispatcher::instance()->deregisterHandler( InterruptDispatcher::SPI1Rx );
#endif
    delete( m_spiController );
    delete( m_pic32PrecisionTimerFactory );
    delete( m_absoluteTimer );
    delete( m_bus );
    InterruptDispatcher::s_graphicsDriver = nullptr;
}

void BoopieOnline::_setMembersNull()
{
    m_bus = nullptr;
    m_absoluteTimer = nullptr;
    m_pic32PrecisionTimerFactory = nullptr;
    m_spiController = nullptr;
    m_spiRequester = nullptr;
    m_flash = nullptr;
    m_fs = nullptr;
    m_configurationStoreMemory = nullptr;
    m_lineSerial = nullptr;
    m_assetLoaderEncryptedFactory = nullptr;
    m_assetLoaderBackingFactory = nullptr;
    m_assetCache = nullptr;
    m_programAssetLoaderEncryptedFactory = nullptr;
    m_programAssetLoaderBackingFactory = nullptr;
    m_programAssetCache = nullptr;
    m_telegramAssetLoaderBackingFactory = nullptr;
    m_sceneLoaderBackingFactory = nullptr;
    m_presenceLoaderBackingFactory = nullptr;
    m_midiSerial = nullptr;
    m_miniMapAssetLoaderBackingFactory = nullptr;
    m_miniMapAssetCache = nullptr;
}

void BoopieOnline::getMachineID()
{
    m_machineID = DEVSN0;
}

void BoopieOnline::buildTimerFactory()
{
    m_bus = new BusController;
    m_absoluteTimer = new Timers::PIC32Absolute( m_bus );
    m_timerFactory = new Timers::Factories::PIC32TimerFactory;
    m_pic32PrecisionTimerFactory = new Timers::Factories::PIC32PrecisionTimerFactory;
}

void BoopieOnline::buildPerformanceTimerFactory()
{
    m_performanceTimerFactory = new Timers::Factories::PIC32TimerFactory;
}

void BoopieOnline::buildConfigurationStore()
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
    m_flash = new Memories::SPIFlash( *m_spiController, *m_bus );
    LOG_DEBUG( "Initialising filesystem" );
    m_fs = new KiamaFS( *m_flash );
    LOG_DEBUG( "Done." );
    m_configurationStoreMemory = new Memories::KiamaFS( *m_fs, "config.dat" );

    m_configurationStore = new ConfigurationStore( *m_configurationStoreMemory );
}

void BoopieOnline::buildGraphicsDriver()
{
    m_graphicsDriver = new GraphicsDrivers::RA8873( *m_bus, *m_pic32PrecisionTimerFactory );
    InterruptDispatcher::s_graphicsDriver = m_graphicsDriver;
}

void BoopieOnline::buildPlatform()
{
    m_platform = new Platforms::Boopie( *m_graphicsDriver, *m_bus, *m_spiRequester, *m_timerFactory, m_fs );
}

void BoopieOnline::buildLineDriver()
{
    m_lineSerial = new PICSerial( 1, 800000, 512, 512, true, true ); // true = Enable RTS/CTS flow control.
#if defined(__32MX470F512H__)
    Agape::InterruptDispatcher::instance()->registerHandler( Agape::InterruptDispatcher::UART1, m_lineSerial );
#elif defined(__32MZ2048EFG064__)
    Agape::InterruptDispatcher::instance()->registerHandler( Agape::InterruptDispatcher::UART1Tx, m_lineSerial );
    Agape::InterruptDispatcher::instance()->registerHandler( Agape::InterruptDispatcher::UART1Rx, m_lineSerial );
#endif
    m_lineDriver = new LineDrivers::PICSerial( *m_lineSerial, *m_timerFactory );
}

void BoopieOnline::buildLine()
{
    m_line = new Lines::Modem( *m_lineDriver );
}

void BoopieOnline::buildInputDevice()
{
    m_inputDevice = new InputDevices::SPIKeyboard( *m_spiRequester, *m_timerFactory );

    m_memoryStrategy = new UI::Strategies::Memory( *m_windowManager,
                                                   _Map,
                                                   *m_inputDevice,
                                                   *m_flash,
                                                   m_debugSerial,
                                                   *m_pic32PrecisionTimerFactory );
}

void BoopieOnline::buildTupleRoute()
{
    m_rwBuffer = new RWBuffer( 512, *m_line );
    m_tupleRoute = new TupleRoutes::ReadableWritable( "", *m_rwBuffer );
}

void BoopieOnline::buildClock()
{
    m_clock = new Clocks::VRTime( *m_vrTime );
    m_eventClock = new EventClocks::Null( *m_tupleRouter );
}

void BoopieOnline::buildEntropySource()
{
    m_entropySource = new EntropySources::SPI( *m_spiRequester, *m_timerFactory );
}

void BoopieOnline::buildAssetLoaderFactory()
{
    m_assetLoaderBackingFactory = new AssetLoaders::Factories::Linda2( *m_tupleRouter,
                                                                       *m_timerFactory,
                                                                       "Assets" );
    m_assetLoaderEncryptedFactory = new AssetLoaders::Factories::Encrypted( *m_assetLoaderBackingFactory,
                                                                            *m_blockEncryptorFactory,
                                                                            *m_worldMetadata,
                                                                            _sharedAssetsWorldID,
                                                                            (char*)_sharedAssetsItemKey,
                                                                            m_encryptorFactory,
                                                                            m_hash,
                                                                            true ); // true = encrypt asset names.
    m_assetCache = new AssetLoaders::Caches::KiamaFSAssetCache( 50,
                                                                10000,
                                                                *m_fs,
                                                                "ans",
                                                                *m_hash,
                                                                *m_clock );
    m_assetLoaderFactory = new AssetLoaders::Factories::Cache( *m_assetLoaderEncryptedFactory,
                                                               *m_assetCache,
                                                               false );
}

void BoopieOnline::buildProgramAssetLoaderFactory()
{
    m_programAssetLoaderBackingFactory = new AssetLoaders::Factories::Linda2( *m_tupleRouter,
                                                                       *m_timerFactory,
                                                                       "Programs" );
    m_programAssetLoaderEncryptedFactory = new AssetLoaders::Factories::Encrypted( *m_programAssetLoaderBackingFactory,
                                                                                   *m_blockEncryptorFactory,
                                                                                   *m_worldMetadata,
                                                                                   _sharedAssetsWorldID,
                                                                                   (char*)_sharedAssetsItemKey,
                                                                                   m_encryptorFactory,
                                                                                   m_hash,
                                                                                   true ); // true = encrypt asset names.
    m_programAssetCache = new AssetLoaders::Caches::KiamaFSAssetCache( 50,
                                                                       10000,
                                                                       *m_fs,
                                                                       "cl2",
                                                                       *m_hash,
                                                                       *m_clock );
    m_programAssetLoaderFactory = new AssetLoaders::Factories::Cache( *m_programAssetLoaderEncryptedFactory,
                                                                      *m_programAssetCache,
                                                                      false );
}

void BoopieOnline::buildTelegramAssetLoaderFactory()
{
    m_telegramAssetLoaderBackingFactory = new AssetLoaders::Factories::Linda2( *m_tupleRouter,
                                                                               *m_timerFactory,
                                                                               "TelegramAssets" );
    m_telegramAssetLoaderFactory = new AssetLoaders::Factories::Encrypted( *m_telegramAssetLoaderBackingFactory,
                                                                           *m_blockEncryptorFactory,
                                                                           *m_worldMetadata );
}

void BoopieOnline::buildMIDIAssetLoaderFactory()
{
    m_midiAssetLoaderFactory = new AssetLoaders::Factories::Baked();
}

void BoopieOnline::buildSceneLoaderFactory()
{
    m_sceneLoaderBackingFactory = new SceneLoaders::Factories::Linda2( *m_tupleRouter,
                                                                       *m_timerFactory,
                                                                       true ); // true = attributes have encrypted names.
    m_sceneLoaderFactory = new SceneLoaders::Factories::Encrypted( *m_sceneLoaderBackingFactory,
                                                                   *m_worldMetadata,
                                                                   *m_encryptorFactory,
                                                                   *m_hash );
}

void BoopieOnline::buildPresenceLoaderFactory()
{
    m_presenceLoaderBackingFactory = new PresenceLoaders::Factories::Linda2( *m_tupleRouter, *m_timerFactory );
    m_presenceLoaderFactory = new PresenceLoaders::Factories::Encrypted( *m_presenceLoaderBackingFactory,
                                                                         *m_worldMetadata,
                                                                         *m_encryptorFactory );
}

void BoopieOnline::buildTelegramLoaderFactory()
{
    m_telegramLoaderFactory = new TelegramLoaders::Factories::Linda2( *m_tupleRouter, *m_timerFactory );
}

void BoopieOnline::buildMIDIPlayer()
{
    m_midiSerial = new PICSerial( 3, 31250, 128, 16 );
#if defined(__32MX470F512H__)
    Agape::InterruptDispatcher::instance()->registerHandler( Agape::InterruptDispatcher::UART3, m_midiSerial );
#elif defined(__32MZ2048EFG064__)
    Agape::InterruptDispatcher::instance()->registerHandler( Agape::InterruptDispatcher::UART3Tx, m_midiSerial );
    Agape::InterruptDispatcher::instance()->registerHandler( Agape::InterruptDispatcher::UART3Rx, m_midiSerial );
#endif
    m_midiPlayer = new Audio::MIDIPlayers::SAM2695( *m_midiAssetLoaderFactory, *m_midiSerial );
}

void BoopieOnline::buildWorldLoaderFactory()
{
    m_worldLoaderFactory = new WorldLoaders::Factories::Linda2( *m_tupleRouter, *m_timerFactory );
}

void BoopieOnline::buildSplash()
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

void BoopieOnline::buildMiniMapAssetLoaderFactory()
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

} // namespace ClientBuilders

} // namespace Agape
