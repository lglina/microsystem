#ifndef AGAPE_WORLD_LOADERS_MONGO_H
#define AGAPE_WORLD_LOADERS_MONGO_H

#include "World/WorldMetadata.h"
#include "String.h"
#include "WorldLoader.h"

#include <functional>

namespace Agape
{

namespace Stratus
{
class Authenticator;
} // namespace Stratus

namespace WorldLoaders
{

class Mongo : public WorldLoader
{
public:
    Mongo( Stratus::Authenticator& authenticator );

    virtual bool create( const World::Metadata& metadata, String& reason );
    virtual bool join( World::Metadata& metadata, String& reason );
    virtual bool load( World::Metadata& metadata, String& reason );

    virtual bool loadJoinedWorlds( Vector< World::Metadata >& joinedWorlds, bool allDevices, String& reason );
    
    virtual bool loadTeleports( Vector< World::Teleport >& teleports, bool allDevices, String& reason );
    virtual bool createTeleport( World::Teleport& teleport, String& reason );
    virtual bool deleteTeleport( World::Teleport& teleport, String& reason );

    virtual bool loadWorldSummaries( Vector< World::Summary >& worldSummaries, int from, int size, String& reason );

    virtual bool loadUniverseStats( World::UniverseStats& universeStats, String& reason );

private:
    bool addJoinedWorld( const String& worldID,
                         const String& privateKey,
                         const World::User& user,
                         String& reason );
    bool withDevice( const String& accountAuthKeyHash,
                     const String& deviceAuthKeyHash,
                     bool allDevices,
                     String& reason,
                     std::function< bool( const Value*, String& ) > deviceCallback );
    bool getDeviceJoinedWorlds( Vector< World::Metadata >& joinedWorlds, const Value* deviceValue, String& reason );
    bool getDeviceTeleports( Vector< World::Teleport >& teleports, const Value* deviceValue, String& reason );

    bool worldUserCount( const String& worldID, int& count, String& reason );
    bool worldItemCount( const String& worldID, int& count, String& reason );
    
    Stratus::Authenticator& m_authenticator;
};

} // namespace WorldLoaders

} // namespace Agape

#endif // AGAPE_WORLD_LOADERS_MONGO_H
