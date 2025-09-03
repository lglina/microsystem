#include "Encryptors/Factories/EncryptorsFactory.h"
#include "Encryptors/Encryptor.h"
#include "PresenceLoaders/Factories/PresenceLoadersFactory.h"
#include "World/ScenePresence.h"
#include "World/WorldMetadata.h"
#include "Collections.h"
#include "EncryptedPresenceLoader.h"
#include "PresenceLoader.h"
#include "PresenceRequest.h"

using namespace Agape::World;

namespace Agape
{

namespace PresenceLoaders
{

Encrypted::Encrypted( const World::Coordinates& coordinates,
                      bool receiveRequests,
                      Factory& backingLoaderFactory,
                      Metadata& worldMetadata,
                      Encryptors::Factory& encryptorFactory ) :
  PresenceLoader( coordinates ),
  m_worldMetadata( worldMetadata ),
  m_encryptor( encryptorFactory.makeEncryptor() ),
  m_backingLoader( backingLoaderFactory.makeLoader( coordinates, receiveRequests ) )
{
}

Encrypted::~Encrypted()
{
    delete( m_encryptor );
    delete( m_backingLoader );
}

bool Encrypted::load( Vector< ScenePresence >& scenePresences )
{
    if( m_backingLoader->load( scenePresences ) )
    {
        return decryptPresences( scenePresences );
    }

    return false;
}

bool Encrypted::loadWorld( Vector< ScenePresence >& worldPresences )
{
    if( m_backingLoader->loadWorld( worldPresences ) )
    {
        return decryptPresences( worldPresences );
    }

    return false;
}

bool Encrypted::request( const Vector< PresenceRequest >& requests )
{
    m_encryptor->setKey( &m_worldMetadata.m_itemKey[0] );
    Vector< PresenceRequest > encryptedRequests( requests );
    Vector< PresenceRequest >::iterator it( encryptedRequests.begin() );
    for( ; it != encryptedRequests.end(); ++it )
    {
        if( !it->encrypt( *m_encryptor ) )
        {
            return false;
        }
    }

    return( m_backingLoader->request( encryptedRequests ) );
}

Vector< PresenceRequest > Encrypted::getUpdates()
{
    m_encryptor->setKey( &m_worldMetadata.m_itemKey[0] );
    Vector< PresenceRequest > encryptedRequests( m_backingLoader->getUpdates() );
    Vector< PresenceRequest >::iterator it( encryptedRequests.begin() );
    for( ; it != encryptedRequests.end(); ++it )
    {
        if( !it->decrypt( *m_encryptor ) )
        {
            break;
        }
    }

    return encryptedRequests;
}

bool Encrypted::overflowed()
{
    return m_backingLoader->overflowed();
}

bool Encrypted::decryptPresences( Vector< ScenePresence >& scenePresences )
{
    m_encryptor->setKey( &m_worldMetadata.m_itemKey[0] );
    Vector< ScenePresence >::iterator it( scenePresences.begin() );
    for( ; it != scenePresences.end(); ++it )
    {
        if( !it->decrypt( *m_encryptor ) )
        {
            return false;
        }
    }

    return true;
}

} // namespace PresenceLoaders

} // namespace Agape
