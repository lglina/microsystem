#include "Encryptors/Factories/EncryptorsFactory.h"
#include "Encryptors/Encryptor.h"
#include "Utils/base64/base64.h"
#include "Utils/Snowflake.h"
#include "KeyUtilities.h"
#include "String.h"
#include "User.h"
#include "WorldMetadata.h"
#include "WorldUtilities.h"

using Agape::String;

namespace Agape
{

namespace World
{

Utilities::Utilities( KeyUtilities& keyUtilities,
                      Snowflake& snowflake,
                      Encryptors::Factory& encryptorFactory ) :
  m_keyUtilities( keyUtilities ),
  m_snowflake( snowflake ),
  m_encryptor( encryptorFactory.makeEncryptor() )
{
}

Utilities::~Utilities()
{
    delete( m_encryptor );
}

void Utilities::generateWorldKey( char* worldKey )
{
    m_keyUtilities.generateWorldKey( worldKey );
}

int Utilities::worldKeySize()
{
    return m_keyUtilities.worldKeySize();
}

void Utilities::generateCreateMetadata( const char* worldKey,
                                        const char* accountSubKey,
                                        const String& worldName,
                                        const String& userName,
                                        int userGlyphBase,
                                        int userAttributes,
                                        Metadata& plainMetadata,
                                        User& plainUser )
{
    // Write provided name and world key to plainMetadata.
    plainMetadata.m_name = worldName;
    plainMetadata.m_worldKey = String( worldKey, m_keyUtilities.worldKeySize() );
    
    // Generate and write item key.
    plainMetadata.m_itemKey.resize( m_keyUtilities.worldKeySize() );
    m_keyUtilities.generateItemKey( &plainMetadata.m_itemKey[0] );

    // Generate world ID from world key, base64 encode and write.
    char* binWorldID = new char[m_keyUtilities.worldIDSize()];
    m_keyUtilities.getWorldID( worldKey, binWorldID );

    plainMetadata.m_worldID.resize( Base64encode_len( m_keyUtilities.worldIDSize() ), '\0' );
    Base64encode( &plainMetadata.m_worldID[0], binWorldID, m_keyUtilities.worldIDSize() );
    plainMetadata.m_worldID.resize( plainMetadata.m_worldID.length() - 1 );
    delete[]( binWorldID );

    // Generate world auth ID from world key, base64 encode and write.
    char* binWorldAuthKey = new char[m_keyUtilities.worldIDSize()];
    m_keyUtilities.getWorldAuthKey( worldKey, binWorldAuthKey );

    plainMetadata.m_worldAuthKey.resize( Base64encode_len( m_keyUtilities.worldIDSize() ), '\0' );
    Base64encode( &plainMetadata.m_worldAuthKey[0], binWorldAuthKey, m_keyUtilities.worldIDSize() );
    plainMetadata.m_worldAuthKey.resize( plainMetadata.m_worldAuthKey.length() - 1 );
    delete[]( binWorldAuthKey );

    // Encrypt world key with account subkey, base64 encode and write.
    char* binPrivateKey = new char[m_keyUtilities.encryptedWorldKeySize()];
    m_keyUtilities.encryptWorldKey( worldKey, accountSubKey, binPrivateKey );

    plainMetadata.m_privateKey.resize( Base64encode_len( m_keyUtilities.encryptedWorldKeySize() ), '\0' );
    Base64encode( &plainMetadata.m_privateKey[0], binPrivateKey, m_keyUtilities.encryptedWorldKeySize() );
    plainMetadata.m_privateKey.resize( plainMetadata.m_privateKey.length() - 1 );
    delete[]( binPrivateKey );

    // Write user data.
    plainUser.m_snowflake = m_snowflake.generate();
    plainUser.m_name = userName;
    plainUser.m_glyph = userGlyphBase;
    plainUser.m_attributes = userAttributes;

    plainMetadata.m_users.push_back( plainUser );
}

void Utilities::generateJoinMetadata( const char* worldKey,
                                      const char* accountSubKey,
                                      const String& userName,
                                      int userGlyphBase,
                                      int userAttributes,
                                      Metadata& plainMetadata,
                                      User& plainUser )
{
    // Generate world auth ID from world key, base64 encode and write.
    char* binWorldAuthKey = new char[m_keyUtilities.worldIDSize()];
    m_keyUtilities.getWorldAuthKey( worldKey, binWorldAuthKey );

    plainMetadata.m_worldAuthKey.resize( Base64encode_len( m_keyUtilities.worldIDSize() ), '\0' );
    Base64encode( &plainMetadata.m_worldAuthKey[0], binWorldAuthKey, m_keyUtilities.worldIDSize() );
    plainMetadata.m_worldAuthKey.resize( plainMetadata.m_worldAuthKey.length() - 1 );
    delete[]( binWorldAuthKey );

    // Encrypt world key with account subkey, base64 encode and write.
    char* binPrivateKey = new char[m_keyUtilities.encryptedWorldKeySize()];
    m_keyUtilities.encryptWorldKey( worldKey, accountSubKey, binPrivateKey );

    plainMetadata.m_privateKey.resize( Base64encode_len( m_keyUtilities.encryptedWorldKeySize() ), '\0' );
    Base64encode( &plainMetadata.m_privateKey[0], binPrivateKey, m_keyUtilities.encryptedWorldKeySize() );
    plainMetadata.m_privateKey.resize( plainMetadata.m_privateKey.length() - 1 );
    delete[]( binPrivateKey );

    // Write user data.
    plainUser.m_snowflake = m_snowflake.generate();
    plainUser.m_name = userName;
    plainUser.m_glyph = userGlyphBase;
    plainUser.m_attributes = userAttributes;

    plainMetadata.m_users.push_back( plainUser );
}

void Utilities::encryptMetadata( const char* worldKey,
                                 const Metadata& plainMetadata,
                                 Metadata& encryptedMetadata )
{
    encryptedMetadata = plainMetadata;
    m_encryptor->setKey( worldKey );
    encryptedMetadata.encrypt( *m_encryptor );
}

void Utilities::decryptMetadata( const char* worldKey,
                                 const Metadata& encryptedMetadata,
                                 Metadata& plainMetadata )
{
    plainMetadata = encryptedMetadata;
    m_encryptor->setKey( worldKey );
    plainMetadata.decrypt( *m_encryptor );
}

void Utilities::decryptPrivateKey( const Metadata& metadata,
                                   const char* accountSubKey,
                                   char* worldKey )
{
    String encryptedWorldKey( Base64decode_len( metadata.m_privateKey.c_str() ), '\0' );
    encryptedWorldKey.resize( Base64decode( &encryptedWorldKey[0], metadata.m_privateKey.c_str() ) );
    m_keyUtilities.decryptWorldKey( encryptedWorldKey.c_str(),
                                    accountSubKey,
                                    worldKey );
}

} // namespace World

} // namespace Agape
