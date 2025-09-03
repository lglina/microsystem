#ifndef AGAPE_CLIENT_BUILDERS_SIMULATED_ONLINE_H
#define AGAPE_CLIENT_BUILDERS_SIMULATED_ONLINE_H

#include "ClientBuilder.h"

namespace Agape
{

namespace AssetLoaders
{
class AssetCache;
class Factory;
} // namespace AssetLoaders

class KiamaFS;
class Memory;

namespace ClientBuilders
{

class SimulatedOnline : public ClientBuilder
{
public:
    SimulatedOnline();
    ~SimulatedOnline();

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

    Memory* m_configurationStoreMemory;

    Memory* m_fsMemory;
    KiamaFS* m_fs;

    AssetLoaders::Factory* m_assetLoaderEncryptedFactory;
    AssetLoaders::Factory* m_assetLoaderBackingFactory;
    AssetLoaders::AssetCache* m_assetCache;
    AssetLoaders::Factory* m_programAssetLoaderEncryptedFactory;
    AssetLoaders::Factory* m_programAssetLoaderBackingFactory;
    AssetLoaders::AssetCache* m_programAssetCache;
    AssetLoaders::Factory* m_telegramAssetLoaderBackingFactory;
    SceneLoaders::Factory* m_sceneLoaderBackingFactory;
    PresenceLoaders::Factory* m_presenceLoaderBackingFactory;
    AssetLoaders::Factory* m_miniMapAssetLoaderBackingFactory;
    AssetLoaders::AssetCache* m_miniMapAssetCache;
};

} // namespace ClientBuilders

} // namespace Agape

#endif // AGAPE_CLIENT_BUILDERS_SIMULATED_ONLINE_H
