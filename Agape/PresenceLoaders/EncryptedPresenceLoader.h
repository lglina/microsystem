#ifndef AGAPE_PRESENCE_LOADERS_ENCRYPTED_H
#define AGAPE_PRESENCE_LOADERS_ENCRYPTED_H

#include "World/ScenePresence.h"
#include "Collections.h"
#include "PresenceLoader.h"
#include "PresenceRequest.h"

using namespace Agape::World;

namespace Agape
{

namespace Encryptors
{
class Factory;
} // namespace Encryptors

namespace World
{
class Coordinates;
class Metadata;
} // namespace World

class Encryptor;

namespace PresenceLoaders
{

class Factory;

class Encrypted : public PresenceLoader
{
public:
    Encrypted( const World::Coordinates& coordinates,
               bool receiveRequests,
               Factory& backingLoaderFactory,
               Metadata& worldMetadata,
               Encryptors::Factory& encryptorFactory );
    ~Encrypted();

    virtual bool load( Vector< ScenePresence >& scenePresences );

    virtual bool loadWorld( Vector< ScenePresence >& worldPresences );

    virtual bool request( const Vector< PresenceRequest >& requests );
    virtual Vector< PresenceRequest > getUpdates();

    virtual bool overflowed();

private:
    bool decryptPresences( Vector< ScenePresence >& scenePresences );

    Metadata& m_worldMetadata;
    
    Encryptor* m_encryptor;

    PresenceLoader* m_backingLoader;
};

} // namespace PresenceLoaders

} // namespace Agape

#endif // AGAPE_PRESENCE_LOADERS_ENCRYPTED_H
