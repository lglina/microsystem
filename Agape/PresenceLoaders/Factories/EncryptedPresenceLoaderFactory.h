#ifndef AGAPE_PRESENCE_LOADERS_FACTORIES_ENCRYPTED_H
#define AGAPE_PRESENCE_LOADERS_FACTORIES_ENCRYPTED_H

#include "PresenceLoaders/EncryptedPresenceLoader.h"
#include "PresenceLoadersFactory.h"

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

namespace PresenceLoaders
{

namespace Factories
{

class Encrypted : public Factory
{
public:
    Encrypted( Factory& backingLoaderFactory,
               Metadata& worldMetadata,
               Encryptors::Factory& encryptorFactory );

    virtual PresenceLoader* makeLoader( const World::Coordinates& coordinates, bool receiveRequests );

private:
    Factory& m_backingLoaderFactory;
    Metadata& m_worldMetadata;
    Encryptors::Factory& m_encryptorFactory;
};

} // namespace Factories

} // namespace PresenceLoaders

} // namespace Agape

#endif // AGAPE_PRESENCE_LOADERS_FACTORIES_ENCRYPTED_H
