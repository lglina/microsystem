#include "AssetLoaders/Factories/FileAssetLoaderFactory.h"
#include "AssetLoaders/Factories/MiniMapAssetLoaderFactory.h"
#include "Audio/MIDIPlayers/ALSAMIDIPlayer.h"
#include "Audio/MIDIPlayers/NullMIDIPlayer.h"
#include "Clocks/VRTimeClock.h"
#include "EntropySources/CRand.h"
#include "EventClocks/CEventClock.h"
#include "GraphicsDrivers/QtWindGraphicsDriver.h"
#include "InputDevices/QtWindInputDevice.h"
#include "LineDrivers/NullLineDriver.h"
#include "Lines/DummyModemLine.h"
#include "Loggers/Logger.h"
#include "Memories/FileMemory.h"
#include "Memories/KiamaFSMemory.h"
#include "Platforms/SimulatedPlatform.h"
#include "PresenceLoaders/Factories/Linda2PresenceLoaderFactory.h"
#include "PresenceLoaders/Factories/OfflinePresenceLoaderFactory.h"
#include "PresenceLoaders/Linda2PresenceLoaderResponder.h"
#include "PresenceLoaders/OfflinePresenceStore.h"
#include "SceneLoaders/Factories/FileSceneLoaderFactory.h"
#include "SceneLoaders/Factories/Linda2SceneLoaderFactory.h"
#include "SceneLoaders/Linda2SceneLoaderResponder.h"
#include "TelegramLoaders/Factories/FileTelegramLoaderFactory.h"
#include "Timers/Factories/CTimerFactory.h"
#include "Timers/Factories/HighResTimerFactory.h"
#include "TupleRoutes/NullTupleRoute.h"
#include "UI/Strategies/Splash.h"
#include "WorldLoaders/Factories/FileWorldLoaderFactory.h"
#include "ConfigurationStore.h"
#include "SimulatedOfflineClientBuilder.h"
#include "TupleRouter.h"

#include <QObject>

#include <stdlib.h>
#include <time.h>

namespace Agape
{

namespace ClientBuilders
{

SimulatedOffline::SimulatedOffline()
{
    _setMembersNull();
}

SimulatedOffline::~SimulatedOffline()
{
    _deleteMembers();
}

void SimulatedOffline::_unbuild()
{
    LOG_DEBUG( "SimulatedOfflineClientBuilder: Unbuilding" );
    _deleteMembers();
    _setMembersNull();
}

void SimulatedOffline::_deleteMembers()
{
    LOG_DEBUG( "SimulatedOfflineClientBuilder: Deleting members" );
    delete( m_presenceLoaderResponder );
    delete( m_presenceLoaderBackingFactory );
    delete( m_offlinePresenceStore );
    delete( m_sceneLoaderResponder );
    delete( m_sceneLoaderBackingFactory );
    delete( m_configurationStoreMemory );
}

void SimulatedOffline::_setMembersNull()
{
    LOG_DEBUG( "SimulatedOfflineClientBuilder: Setting members to nullptr" );
    m_configurationStoreMemory = nullptr;
    m_sceneLoaderBackingFactory = nullptr;
    m_sceneLoaderResponder = nullptr;
    m_offlinePresenceStore = nullptr;
    m_presenceLoaderBackingFactory = nullptr;
    m_presenceLoaderResponder = nullptr;
}

void SimulatedOffline::getMachineID()
{
    srand( time( NULL ) );
    m_machineID = rand();
}

void SimulatedOffline::buildTimerFactory()
{
    m_timerFactory = new Timers::Factories::C;
}

void SimulatedOffline::buildPerformanceTimerFactory()
{
    m_performanceTimerFactory = new Timers::Factories::HighRes;
}

void SimulatedOffline::buildConfigurationStore()
{
    m_configurationStoreMemory = new Memories::File( "config-offline.dat", Memory::eeprom, 0x10000 );
    m_configurationStore = new ConfigurationStore( *m_configurationStoreMemory );
}

void SimulatedOffline::buildGraphicsDriver()
{
    m_graphicsDriver = new GraphicsDrivers::QtWind( *m_timerFactory );
}

void SimulatedOffline::buildPlatform()
{
    m_platform = new Platforms::Simulated( *m_timerFactory );
}

void SimulatedOffline::buildLineDriver()
{
    m_lineDriver = new LineDrivers::Null;
}

void SimulatedOffline::buildLine()
{
    m_line = new Lines::DummyModem( *m_lineDriver,
                                    *m_configurationStore,
                                    *m_timerFactory );
}

void SimulatedOffline::buildInputDevice()
{
    m_inputDevice = new InputDevices::QtWind;
    QObject::connect( (GraphicsDrivers::QtWind*)m_graphicsDriver,
                      &GraphicsDrivers::QtWind::keyPressed,
                      (InputDevices::QtWind*)m_inputDevice,
                      &InputDevices::QtWind::consumeKeyPress );
}

void SimulatedOffline::buildTupleRoute()
{
    m_tupleRoute = new Linda2::TupleRoutes::Null( "Dummy" );
}

void SimulatedOffline::buildClock()
{
    m_clock = new Clocks::VRTime( *m_vrTime );
    m_eventClock = new EventClocks::C( *m_tupleRouter );
}

void SimulatedOffline::buildEntropySource()
{
    // FIXME: Pseudo-random
    m_entropySource = new EntropySources::CRand;
}

void SimulatedOffline::buildAssetLoaderFactory()
{
    m_assetLoaderFactory = new AssetLoaders::Factories::File( "Assets", "ans" );
}

void SimulatedOffline::buildProgramAssetLoaderFactory()
{
    m_programAssetLoaderFactory = new AssetLoaders::Factories::File( "Programs", "cl2" );
}

void SimulatedOffline::buildTelegramAssetLoaderFactory()
{
    m_telegramAssetLoaderFactory = new AssetLoaders::Factories::File( "Telegrams", "txt" );
}

void SimulatedOffline::buildMIDIAssetLoaderFactory()
{
    m_midiAssetLoaderFactory = new AssetLoaders::Factories::File( "Sounds", "fmid" );
}

void SimulatedOffline::buildSceneLoaderFactory()
{
    m_sceneLoaderFactory = new SceneLoaders::Factories::Linda2( *m_tupleRouter, *m_timerFactory );
    m_sceneLoaderBackingFactory = new SceneLoaders::Factories::File( "Scenes", "scn", "Attributes", "sia" );
    m_sceneLoaderResponder = new SceneLoaders::Linda2Responder( *m_tupleRouter, *m_sceneLoaderBackingFactory );
}

void SimulatedOffline::buildPresenceLoaderFactory()
{
    m_presenceLoaderFactory = new PresenceLoaders::Factories::Linda2( *m_tupleRouter, *m_timerFactory );
    m_offlinePresenceStore = new PresenceLoaders::OfflinePresenceStore();
    m_presenceLoaderBackingFactory = new PresenceLoaders::Factories::Offline( *m_offlinePresenceStore, *m_clock );
    m_presenceLoaderResponder = new PresenceLoaders::Linda2Responder( *m_tupleRouter, *m_presenceLoaderBackingFactory );
}

void SimulatedOffline::buildTelegramLoaderFactory()
{
    m_telegramLoaderFactory = new TelegramLoaders::Factories::File( "telegrams.dat", *m_worldMetadata );
}

void SimulatedOffline::buildMIDIPlayer()
{
#ifndef _WIN32
    m_midiPlayer = new Audio::MIDIPlayers::ALSA( *m_midiAssetLoaderFactory );
#else
    m_midiPlayer = new Audio::MIDIPlayers::Null;
#endif
}

void SimulatedOffline::buildWorldLoaderFactory()
{
    m_worldLoaderFactory = new WorldLoaders::Factories::File( "Worlds", "wld" );
}

void SimulatedOffline::buildSplash()
{
    m_splash = new UI::Strategies::Splash( *m_windowManager,
                                           *m_inputDevice,
                                           *m_timerFactory,
                                           *m_platform );
}

void SimulatedOffline::buildMiniMapAssetLoaderFactory()
{
    m_miniMapAssetLoaderFactory = new AssetLoaders::Factories::MiniMap( *m_miniMap );
}

} // namespace ClientBuilders

} // namespace Agape
