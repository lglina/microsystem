#ifndef AGAPE_ASSET_LOADERS_FACTORIES_MONGO_H
#define AGAPE_ASSET_LOADERS_FACTORIES_MONGO_H

#include "AssetLoaders/AssetLoader.h"
#include "AssetLoadersFactory.h"
#include "String.h"

namespace Agape
{

namespace Stratus
{
class Authenticator;
} // namespace Stratus

namespace World
{
class Coordinates;
} // namespace World

using namespace Stratus;

namespace AssetLoaders
{

namespace Factories
{

class Mongo : public Factory
{
public:
    Mongo( const String& collectionName,
           bool encryptedNames,
           Authenticator& authenticator );

    virtual AssetLoader* makeLoader( const World::Coordinates& coordinates,
                                     const String& name );

private:
    String m_collectionName;
    bool m_encryptedNames;
    Authenticator& m_authenticator;
};

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape

#endif // AGAPE_ASSET_LOADERS_FACTORIES_MONGO_H
