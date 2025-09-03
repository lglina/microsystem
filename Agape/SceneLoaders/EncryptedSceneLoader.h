#ifndef AGAPE_SCENE_LOADERS_ENCRYPTED_H
#define AGAPE_SCENE_LOADERS_ENCRYPTED_H

#include "Collections.h"
#include "SceneLoader.h"
#include "SceneRequest.h"
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
class Scene;
} // namespace World

class Encryptor;
class Hash;

namespace SceneLoaders
{

class Factory;

class Encrypted : public SceneLoader
{
public:
    Encrypted( const World::Coordinates& coordinates,
               Factory& backingLoaderFactory,
               Metadata& worldMetadata,
               Encryptors::Factory& encryptorFactory,
               Hash& hash );
    ~Encrypted();

    virtual bool load( World::Scene& scene );
    virtual bool request( const Vector< SceneRequest >& requests );
    virtual Vector< SceneRequest > getUpdates();

    virtual bool hasSceneItemAttribute( const String& snowflake,
                                        const String& name );
    virtual bool createSceneItemAttribute( const String& snowflake,
                                           const String& name );
    virtual bool loadSceneItemAttribute( const String& snowflake,
                                         const String& name,
                                         Value& value );
    virtual bool saveSceneItemAttribute( const String& snowflake,
                                         const String& name,
                                         const Value& value );
    virtual bool deleteSceneItemAttributes( const String& snowflake );

    virtual void invalidateCachedAsset( const struct InvalidatedAsset& invalidatedAsset );
    virtual Vector< struct InvalidatedAsset > getInvalidatedAssets();

private:
    String encryptedAttributeName( const char* itemKey,
                                   const String& attributeNamePlain );

    Metadata& m_worldMetadata;
    Hash& m_hash;

    SceneLoader* m_backingLoader;
    Encryptor* m_encryptor;
};

} // namespace SceneLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_ENCRYPTED_H
