#include "AssetLoaders/Caches/KiamaFSAssetCache.h"
#include "AssetLoaders/Factories/CacheAssetLoaderFactory.h"
#include "AssetLoaders/Factories/EncryptedAssetLoaderFactory.h"
#include "AssetLoaders/Factories/FileAssetLoaderFactory.h"
#include "AssetLoaders/Factories/Linda2AssetLoaderFactory.h"
#include "AssetLoaders/Factories/MiniMapAssetLoaderFactory.h"
#include "AssetLoaders/CacheAssetLoader.h"
#include "Audio/MIDIPlayers/ALSAMIDIPlayer.h"
#include "Audio/MIDIPlayers/NullMIDIPlayer.h"
#include "Clocks/VRTimeClock.h"
#include "Encryptors/AES/AESBlockEncryptor.h"
#include "Encryptors/BlockEncryptor.h"
#include "EntropySources/CRand.h"
#include "EventClocks/NullEventClock.h"
#include "GraphicsDrivers/QtWindGraphicsDriver.h"
#include "InputDevices/QtWindInputDevice.h"
#include "LineDrivers/QtWebSocketsLineDriver.h"
#include "Lines/DummyModemLine.h"
#include "Loggers/Logger.h"
#include "Memories/FileMemory.h"
#include "Memories/KiamaFSMemory.h"
#include "Platforms/SimulatedPlatform.h"
#include "PresenceLoaders/Factories/EncryptedPresenceLoaderFactory.h"
#include "PresenceLoaders/Factories/Linda2PresenceLoaderFactory.h"
#include "SceneLoaders/Factories/EncryptedSceneLoaderFactory.h"
#include "SceneLoaders/Factories/Linda2SceneLoaderFactory.h"
#include "TelegramLoaders/Factories/Linda2TelegramLoaderFactory.h"
#include "Timers/Factories/CTimerFactory.h"
#include "Timers/Factories/HighResTimerFactory.h"
#include "TupleRoutes/ReadableWritableTupleRoute.h"
#include "UI/Strategies/Splash.h"
#include "WorldLoaders/Factories/Linda2WorldLoaderFactory.h"
#include "ConfigurationStore.h"
#include "KiamaFS.h"
#include "RWBuffer.h"
#include "SimulatedOnlineClientBuilder.h"
#include "TupleRouter.h"

#include <QApplication>
#include <QObject>

#include <stdlib.h>
#include <time.h>

namespace Agape
{

namespace ClientBuilders
{

SimulatedOnline::SimulatedOnline()
{
    _setMembersNull();
}

SimulatedOnline::~SimulatedOnline()
{
    _deleteMembers();
}

void SimulatedOnline::_unbuild()
{
    LOG_DEBUG( "SimulatedOnlineClientBuilder: Unbuilding" );
    _deleteMembers();
    _setMembersNull();
}

void SimulatedOnline::_deleteMembers()
{
    LOG_DEBUG( "SimulatedOnlineClientBuilder: Deleting members" );
    delete( m_miniMapAssetCache );
    delete( m_miniMapAssetLoaderBackingFactory );
    delete( m_presenceLoaderBackingFactory );
    delete( m_sceneLoaderBackingFactory );
    delete( m_telegramAssetLoaderBackingFactory );
    delete( m_programAssetCache );
    delete( m_programAssetLoaderBackingFactory );
    delete( m_programAssetLoaderEncryptedFactory );
    delete( m_assetCache );
    delete( m_assetLoaderBackingFactory );
    delete( m_assetLoaderEncryptedFactory );
    delete( m_fs );
    delete( m_fsMemory );
    delete( m_configurationStoreMemory );
}

void SimulatedOnline::_setMembersNull()
{
    LOG_DEBUG( "SimulatedOnlineClientBuilder: Setting members to nullptr" );
    m_configurationStoreMemory = nullptr;
    m_fsMemory = nullptr;
    m_fs = nullptr;
    m_assetLoaderEncryptedFactory = nullptr;
    m_assetLoaderBackingFactory = nullptr;
    m_assetCache = nullptr;
    m_programAssetLoaderEncryptedFactory = nullptr;
    m_programAssetLoaderBackingFactory = nullptr;
    m_programAssetCache = nullptr;
    m_telegramAssetLoaderBackingFactory = nullptr;
    m_sceneLoaderBackingFactory = nullptr;
    m_presenceLoaderBackingFactory = nullptr;
    m_miniMapAssetLoaderBackingFactory = nullptr;
    m_miniMapAssetCache = nullptr;
}

void SimulatedOnline::getMachineID()
{
    srand( time( NULL ) );
    m_machineID = rand();
}

void SimulatedOnline::buildTimerFactory()
{
    m_timerFactory = new Timers::Factories::C;
}

void SimulatedOnline::buildPerformanceTimerFactory()
{
    m_performanceTimerFactory = new Timers::Factories::HighRes;
}

void SimulatedOnline::buildConfigurationStore()
{
    m_configurationStoreMemory = new Memories::File( "config.dat", Memory::eeprom, 0x10000 );
    m_configurationStore = new ConfigurationStore( *m_configurationStoreMemory );
}

void SimulatedOnline::buildGraphicsDriver()
{
    m_graphicsDriver = new GraphicsDrivers::QtWind( *m_timerFactory );
}

void SimulatedOnline::buildPlatform()
{
    m_fsMemory = new Memories::File( "kiamafs.dat", Memory::flash, 0x100000, 0x1000 );
    m_fs = new KiamaFS( *m_fsMemory );
    m_platform = new Platforms::Simulated( *m_timerFactory, m_fs );
}

void SimulatedOnline::buildLineDriver()
{
    m_lineDriver = new LineDrivers::QtWebSockets( *m_platform );
}

void SimulatedOnline::buildLine()
{
    m_line = new Lines::DummyModem( *m_lineDriver,
                                    *m_configurationStore,
                                    *m_timerFactory );
}

void SimulatedOnline::buildInputDevice()
{
    m_inputDevice = new InputDevices::QtWind;
    QObject::connect( (GraphicsDrivers::QtWind*)m_graphicsDriver,
                      &GraphicsDrivers::QtWind::keyPressed,
                      (InputDevices::QtWind*)m_inputDevice,
                      &InputDevices::QtWind::consumeKeyPress );
}

void SimulatedOnline::buildTupleRoute()
{
    m_rwBuffer = new RWBuffer( 512, *m_lineDriver );
    m_tupleRoute = new Linda2::TupleRoutes::ReadableWritable( "Modem", *m_rwBuffer );
}

void SimulatedOnline::buildClock()
{
    m_clock = new Clocks::VRTime( *m_vrTime );
    m_eventClock = new EventClocks::Null( *m_tupleRouter );
}

void SimulatedOnline::buildEntropySource()
{
    // FIXME: Pseudo-random
    m_entropySource = new EntropySources::CRand;
}

void SimulatedOnline::buildAssetLoaderFactory()
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
    m_assetCache = new AssetLoaders::Caches::KiamaFSAssetCache( 100,
                                                                10000,
                                                                *m_fs,
                                                                "ans",
                                                                *m_hash,
                                                                *m_clock );
    m_assetLoaderFactory = new AssetLoaders::Factories::Cache( *m_assetLoaderEncryptedFactory,
                                                               *m_assetCache,
                                                               false );
}

void SimulatedOnline::buildProgramAssetLoaderFactory()
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
    m_programAssetCache = new AssetLoaders::Caches::KiamaFSAssetCache( 100,
                                                                       10000,
                                                                       *m_fs,
                                                                       "cl2",
                                                                       *m_hash,
                                                                       *m_clock );
    m_programAssetLoaderFactory = new AssetLoaders::Factories::Cache( *m_programAssetLoaderEncryptedFactory,
                                                                      *m_programAssetCache,
                                                                      false );
}

void SimulatedOnline::buildTelegramAssetLoaderFactory()
{
    m_telegramAssetLoaderBackingFactory = new AssetLoaders::Factories::Linda2( *m_tupleRouter,
                                                                               *m_timerFactory,
                                                                               "TelegramAssets" );
    m_telegramAssetLoaderFactory = new AssetLoaders::Factories::Encrypted( *m_telegramAssetLoaderBackingFactory,
                                                                           *m_blockEncryptorFactory,
                                                                           *m_worldMetadata );
}

void SimulatedOnline::buildMIDIAssetLoaderFactory()
{
    // FIXME: Load from server.
    m_midiAssetLoaderFactory = new AssetLoaders::Factories::File( "MIDI", "fmid" );
}

void SimulatedOnline::buildSceneLoaderFactory()
{
    m_sceneLoaderBackingFactory = new SceneLoaders::Factories::Linda2( *m_tupleRouter,
                                                                       *m_timerFactory,
                                                                       true ); // true = attributes have encrypted names.
    m_sceneLoaderFactory = new SceneLoaders::Factories::Encrypted( *m_sceneLoaderBackingFactory,
                                                                   *m_worldMetadata,
                                                                   *m_encryptorFactory,
                                                                   *m_hash );
}

void SimulatedOnline::buildPresenceLoaderFactory()
{
    m_presenceLoaderBackingFactory = new PresenceLoaders::Factories::Linda2( *m_tupleRouter, *m_timerFactory );
    m_presenceLoaderFactory = new PresenceLoaders::Factories::Encrypted( *m_presenceLoaderBackingFactory,
                                                                         *m_worldMetadata,
                                                                         *m_encryptorFactory );
}

void SimulatedOnline::buildTelegramLoaderFactory()
{
    m_telegramLoaderFactory = new TelegramLoaders::Factories::Linda2( *m_tupleRouter, *m_timerFactory );
}

void SimulatedOnline::buildMIDIPlayer()
{
#ifndef _WIN32
    m_midiPlayer = new Audio::MIDIPlayers::ALSA( *m_midiAssetLoaderFactory );
#else
    m_midiPlayer = new Audio::MIDIPlayers::Null;
#endif
}

void SimulatedOnline::buildWorldLoaderFactory()
{
    m_worldLoaderFactory = new WorldLoaders::Factories::Linda2( *m_tupleRouter, *m_timerFactory );
}

void SimulatedOnline::buildSplash()
{
    m_splash = new UI::Strategies::Splash( *m_windowManager,
                                           *m_inputDevice,
                                           *m_timerFactory,
                                           *m_platform );
}

void SimulatedOnline::buildMiniMapAssetLoaderFactory()
{
    m_miniMapAssetLoaderBackingFactory = new AssetLoaders::Factories::MiniMap( *m_miniMap );
    m_miniMapAssetCache = new AssetLoaders::Caches::KiamaFSAssetCache( 200,
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
