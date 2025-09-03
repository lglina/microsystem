#ifndef AGAPE_SCENE_LOADERS_FACTORIES_ENCRYPTED_H
#define AGAPE_SCENE_LOADERS_FACTORIES_ENCRYPTED_H

#include "SceneLoadersFactory.h"
#include "String.h"

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

class Hash;
class SceneLoader;

using namespace World;

namespace SceneLoaders
{

namespace Factories
{

class Encrypted : public Factory
{
public:
    Encrypted( Factory& backingLoaderFactory,
               Metadata& worldMetadata,
               Encryptors::Factory& encryptorFactory,
               Hash& hash );

    virtual SceneLoader* makeLoader( const Coordinates& coordinates, bool receiveRequests );

private:
    Factory& m_backingLoaderFactory;
    Metadata& m_worldMetadata;
    Encryptors::Factory& m_encryptorFactory;
    Hash& m_hash;
};

} // namespace Factories

} // namespace SceneLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_FACTORIES_ENCRYPTED_H
