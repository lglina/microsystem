#include "Encryptors/Factories/EncryptorsFactory.h"
#include "Encryptors/Utils/SecureIdentifier.h"
#include "Encryptors/Encryptor.h"
#include "Encryptors/Hash.h"
#include "SceneLoaders/Factories/SceneLoadersFactory.h"
#include "World/Scene.h"
#include "World/SceneItem.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "Collections.h"
#include "EncryptedSceneLoader.h"
#include "SceneRequest.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace SceneLoaders
{

Encrypted::Encrypted( const World::Coordinates& coordinates,
                      Factory& backingLoaderFactory,
                      Metadata& worldMetadata,
                      Encryptors::Factory& encryptorFactory,
                      Hash& hash ) :
  SceneLoader( coordinates ),
  m_worldMetadata( worldMetadata ),
  m_hash( hash ),
  m_backingLoader( backingLoaderFactory.makeLoader( coordinates ) ),
  m_encryptor( encryptorFactory.makeEncryptor() )
{
}

Encrypted::~Encrypted()
{
    delete( m_backingLoader );
    delete( m_encryptor );
}

bool Encrypted::load( World::Scene& scene )
{
    if( m_backingLoader->load( scene ) )
    {
        m_encryptor->setKey( &m_worldMetadata.m_itemKey[0] );
        return( scene.decrypt( *m_encryptor ) );
    }

    return false;
}

bool Encrypted::request( const Vector< SceneRequest >& requests )
{
    m_encryptor->setKey( &m_worldMetadata.m_itemKey[0] );
    Vector< SceneRequest > encryptedRequests( requests );
    Vector< SceneRequest >::iterator it( encryptedRequests.begin() );
    for( ; it != encryptedRequests.end(); ++it )
    {
        if( !it->encrypt( *m_encryptor ) )
        {
            return false;
        }
    }

    return( m_backingLoader->request( encryptedRequests ) );
}

Vector< SceneRequest > Encrypted::getUpdates()
{
    m_encryptor->setKey( &m_worldMetadata.m_itemKey[0] );
    Vector< SceneRequest > encryptedRequests( m_backingLoader->getUpdates() );
    Vector< SceneRequest >::iterator it( encryptedRequests.begin() );
    for( ; it != encryptedRequests.end(); ++it )
    {
        if( !it->decrypt( *m_encryptor ) )
        {
            break;
        }
    }

    return encryptedRequests;
}

bool Encrypted::hasSceneItemAttribute( const String& snowflake,
                                       const String& name )
{
    return( m_backingLoader->hasSceneItemAttribute( snowflake,
                                                    encryptedAttributeName( &m_worldMetadata.m_itemKey[0],
                                                                            name ) ) );
}

bool Encrypted::createSceneItemAttribute( const String& snowflake,
                                          const String& name )
{
    return( m_backingLoader->createSceneItemAttribute( snowflake,
                                                       encryptedAttributeName( &m_worldMetadata.m_itemKey[0],
                                                                               name ) ) );
}

bool Encrypted::loadSceneItemAttribute( const String& snowflake,
                                        const String& name,
                                        Value& value )
{
    Value encryptedValue;
    if( m_backingLoader->loadSceneItemAttribute( snowflake,
                                                 encryptedAttributeName( &m_worldMetadata.m_itemKey[0],
                                                                         name ),
                                                 encryptedValue ) )
    {
        m_encryptor->setKey( &m_worldMetadata.m_itemKey[0] );
        if( encryptedValue.decrypt( *m_encryptor ) )
        {
            value = encryptedValue;
            return true;
        }
    }

    return false;
}

bool Encrypted::saveSceneItemAttribute( const String& snowflake,
                                        const String& name,
                                        const Value& value )
{
    Value encryptedValue( value );
    m_encryptor->setKey( &m_worldMetadata.m_itemKey[0] );
    if( encryptedValue.encrypt( *m_encryptor ) )
    {
        return( m_backingLoader->saveSceneItemAttribute( snowflake,
                                                         encryptedAttributeName( &m_worldMetadata.m_itemKey[0],
                                                                                 name ),
                                                         encryptedValue ) );
    }

    return false;
}

bool Encrypted::deleteSceneItemAttributes( const String& snowflake )
{
    return( m_backingLoader->deleteSceneItemAttributes( snowflake ) );
}

void Encrypted::invalidateCachedAsset( const struct InvalidatedAsset& invalidatedAsset )
{
    m_backingLoader->invalidateCachedAsset( invalidatedAsset );
}

Vector< struct SceneLoader::InvalidatedAsset > Encrypted::getInvalidatedAssets()
{
    return( m_backingLoader->getInvalidatedAssets() );
}

String Encrypted::encryptedAttributeName( const char* itemKey,
                                          const String& attributeNamePlain )
{
    return Encryptors::Utils::SecureIdentifier::createIdentifier( *m_encryptor,
                                                                  m_hash,
                                                                  itemKey,
                                                                  attributeNamePlain );
}

} // namespace SceneLoaders

} // namespace Agape
