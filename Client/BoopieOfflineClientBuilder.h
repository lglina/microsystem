#ifndef AGAPE_CLIENT_BUILDERS_BOOPIE_OFFLINE_H
#define AGAPE_CLIENT_BUILDERS_BOOPIE_OFFLINE_H

#include "ClientBuilder.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{
class AssetCache;
class Factory;
} // namespace AssetLoaders

namespace PresenceLoaders
{
class OfflinePresenceStore;
} // namespace PresenceLoaders

namespace SceneLoaders
{
class Factory;
class Linda2Responder;
} // namespace SceneLoaders

namespace Timers
{
class Factory;
class PIC32Absolute;
} // namespace Timers

class BusController;
class KiamaFS;
class Memory;
class PICSerial;
class SPIController;
class SPIRequester;

namespace ClientBuilders
{

class BoopieOffline : public ClientBuilder
{
public:
    BoopieOffline( ReadableWritable& debugSerial );
    ~BoopieOffline();

private:
    virtual void _unbuild();
    void _deleteMembers();
    void _setMembersNull();

    virtual void getMachineID();
    virtual void buildTimerFactory();
    virtual void buildPerformanceTimerFactory();
    virtual void buildConfigurationStore();
    virtual void buildGraphicsDriver();
    virtual void buildPlatform();
    virtual void buildLineDriver();
    virtual void buildLine();
    virtual void buildInputDevice();
    virtual void buildTupleRoute();
    virtual void buildClock();
    virtual void buildEntropySource();
    virtual void buildAssetLoaderFactory();
    virtual void buildProgramAssetLoaderFactory();
    virtual void buildTelegramAssetLoaderFactory();
    virtual void buildMIDIAssetLoaderFactory();
    virtual void buildSceneLoaderFactory();
    virtual void buildPresenceLoaderFactory();
    virtual void buildTelegramLoaderFactory();
    virtual void buildMIDIPlayer();
    virtual void buildWorldLoaderFactory();
    virtual void buildSplash();
    virtual void buildMiniMapAssetLoaderFactory();

    ReadableWritable& m_debugSerial;

    BusController* m_bus;
    Timers::PIC32Absolute* m_absoluteTimer;
    Timers::Factory* m_pic32PrecisionTimerFactory;

    Memory* m_flash;
    KiamaFS* m_fs;
    Memory* m_configurationStoreMemory;

    SPIController* m_spiController;
    SPIRequester* m_spiRequester;

    SceneLoaders::Factory* m_sceneLoaderBackingFactory;
    SceneLoaders::Linda2Responder* m_sceneLoaderResponder;

    PresenceLoaders::OfflinePresenceStore* m_offlinePresenceStore;

    PICSerial* m_midiSerial;

    AssetLoaders::Factory* m_miniMapAssetLoaderBackingFactory;
    AssetLoaders::AssetCache* m_miniMapAssetCache;
};

} // namespace ClientBuilders

} // namespace Agape

#endif // AGAPE_CLIENT_BUILDERS_BOOPIE_OFFLINE_H
