#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/MongoAssetLoader.h"
#include "World/WorldCoordinates.h"
#include "Authenticator.h"
#include "MongoAssetLoaderFactory.h"
#include "String.h"

namespace Agape
{

using namespace Stratus;

namespace AssetLoaders
{

namespace Factories
{

Mongo::Mongo( const String& collectionName,
              bool encryptedNames,
              Authenticator& authenticator ) :
  m_collectionName( collectionName ),
  m_encryptedNames( encryptedNames ),
  m_authenticator( authenticator )
{
}

AssetLoader* Mongo::makeLoader( const World::Coordinates& coordinates,
                                const String& name )
{
    return new AssetLoaders::Mongo( coordinates, 
                                    name,
                                    m_collectionName,
                                    m_encryptedNames,
                                    m_authenticator );
}

} // namespace Factories

} // namespace AssetLoaders

} // namespace Agape
