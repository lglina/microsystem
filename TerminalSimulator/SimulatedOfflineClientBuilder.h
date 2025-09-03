#ifndef AGAPE_CLIENT_BUILDERS_SIMULATED_OFFLINE_H
#define AGAPE_CLIENT_BUILDERS_SIMULATED_OFFLINE_H

#include "ClientBuilder.h"

namespace Agape
{

namespace AssetLoaders
{
class Factory;
} // namespace AssetLoaders

namespace Linda2
{
class TupleRoute;
} // namespace Linda2

namespace Network
{
class LinuxSocket;
} // namespace Network

namespace PresenceLoaders
{
class Factory;
class Linda2Responder;
class OfflinePresenceStore;
} // namespace PresenceLoaders

namespace SceneLoaders
{
class Factory;
class Linda2Responder;
} // namespace SceneLoaders

namespace WorldLoaders
{
class Factory;
} // namespace WorldLoaders

class EventClock;
class KiamaFS;
class LineDriver;
class Line;
class Memory;

namespace ClientBuilders
{

class SimulatedOffline : public ClientBuilder
{
public:
    SimulatedOffline();
    ~SimulatedOffline();

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

    SceneLoaders::Factory* m_sceneLoaderBackingFactory;
    SceneLoaders::Linda2Responder* m_sceneLoaderResponder;

    PresenceLoaders::OfflinePresenceStore* m_offlinePresenceStore;
    PresenceLoaders::Factory* m_presenceLoaderBackingFactory;
    PresenceLoaders::Linda2Responder* m_presenceLoaderResponder;
};

} // namespace ClientBuilders

} // namespace Agape

#endif // AGAPE_CLIENT_BUILDERS_SIMULATED_OFFLINE_H
