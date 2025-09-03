#ifndef AGAPE_WORLD_LOADER_H
#define AGAPE_WORLD_LOADER_H

#include "Collections.h"
#include "String.h"

namespace Agape
{

namespace World
{
class Metadata;
class Summary;
class Teleport;
class UniverseStats;
} // namespace World

class WorldLoader
{
public:
    virtual ~WorldLoader() {};

    virtual bool create( const World::Metadata& metadata, String& reason ) = 0;
    virtual bool join( World::Metadata& metadata, String& reason ) = 0;
    virtual bool load( World::Metadata& metadata, String& reason ) = 0;

    virtual bool loadJoinedWorlds( Vector< World::Metadata >& joinedWorlds, bool allDevices, String& reason ) { return false; };
    
    virtual bool loadTeleports( Vector< World::Teleport >& teleports, bool allDevices, String& reason ) { return false; };
    virtual bool createTeleport( World::Teleport& teleport, String& reason ) { return false; };
    virtual bool deleteTeleport( World::Teleport& teleport, String& reason ) { return false; };

    virtual bool loadWorldSummaries( Vector< World::Summary >& worldSummaries, int from, int size, String& reason ) { return false; };
    
    virtual bool loadUniverseStats( World::UniverseStats& universeStats, String& reason ) { return false; };
};

} // namespace Agape

#endif // AGAPE_WORLD_LOADER_H
