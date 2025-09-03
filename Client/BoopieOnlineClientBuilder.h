#ifndef AGAPE_CLIENT_BUILDERS_BOOPIE_ONLINE_H
#define AGAPE_CLIENT_BUILDERS_BOOPIE_ONLINE_H

#include "ClientBuilder.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{
class AssetCache;
class Factory;
} // namespace AssetLoaders

namespace Memories
{
class SPIFlash;
} // namespace Memories

namespace PresenceLoaders
{
class Factory;
} // namespace PresenceLoaders

namespace SceneLoaders
{
class Factory;
} // namespace SceneLoaders

namespace Timers
{
class Factory;
class PIC32Absolute;
} // namespace Timers

class BusController;
class KiamaFS;
class PICSerial;
class ReadableWritable;
class SPIController;
class SPIRequester;

namespace ClientBuilders
{

class BoopieOnline : public ClientBuilder
{
public:
    BoopieOnline( ReadableWritable& debugSerial );
    ~BoopieOnline();

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

    SPIController* m_spiController;
    SPIRequester* m_spiRequester;

    Memories::SPIFlash* m_flash;
    KiamaFS* m_fs;

    Memory* m_configurationStoreMemory;

    PICSerial* m_lineSerial;

    AssetLoaders::Factory* m_assetLoaderEncryptedFactory;
    AssetLoaders::Factory* m_assetLoaderBackingFactory;
    AssetLoaders::AssetCache* m_assetCache;
    AssetLoaders::Factory* m_programAssetLoaderEncryptedFactory;
    AssetLoaders::Factory* m_programAssetLoaderBackingFactory;
    AssetLoaders::AssetCache* m_programAssetCache;
    AssetLoaders::Factory* m_telegramAssetLoaderBackingFactory;
    SceneLoaders::Factory* m_sceneLoaderBackingFactory;
    PresenceLoaders::Factory* m_presenceLoaderBackingFactory;
    PICSerial* m_midiSerial;
    AssetLoaders::Factory* m_miniMapAssetLoaderBackingFactory;
    AssetLoaders::AssetCache* m_miniMapAssetCache;
};

} // namespace ClientBuilders

} // namespace Agape

#endif // AGAPE_CLIENT_BUILDERS_BOOPIE_ONLINE_H
