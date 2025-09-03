#include "Encryptors/Factories/EncryptorsFactory.h"
#include "Encryptors/Encryptor.h"
#include "Encryptors/Hash.h"
#include "EntropySources/EntropySource.h"
#include "KeyUtilities.h"

#include <string.h>

namespace
{
    const int _worldKeySize( 16 );
    const int _accountKeySize( 16 );
    const int _deviceAuthKeySize( 16 );
    const int _sealingKeySize( 16 );
} // Anonymous namespace

namespace Agape
{

KeyUtilities::KeyUtilities( EntropySource& entropySource,
                            Encryptors::Factory& encryptorFactory,
                            Hash& hash ) :
  m_entropySource( entropySource ),
  m_hash( hash ),
  m_encryptor( encryptorFactory.makeEncryptor() )
{
}

KeyUtilities::~KeyUtilities()
{
    delete( m_encryptor );
}

int KeyUtilities::worldKeySize()
{
    return _worldKeySize;
}

void KeyUtilities::generateWorldKey( char* worldKey )
{
    m_entropySource.generate( worldKey, _worldKeySize );
}

void KeyUtilities::generateItemKey( char* itemKey )
{
    m_entropySource.generate( itemKey, _worldKeySize );
}

int KeyUtilities::worldIDSize()
{
    return m_hash.digestSize();
}

void KeyUtilities::getWorldID( const char* worldKey, char* worldID )
{
    m_hash.reset();
    m_hash.update( worldKey, _worldKeySize );
    m_hash.finalise( worldID );
}

void KeyUtilities::getWorldAuthKey( const char* worldKey, char* worldAuthKey )
{
    getWorldID( worldKey, worldAuthKey );
}

int KeyUtilities::encryptedWorldKeySize()
{
    return( _worldKeySize + m_encryptor->overhead() );
}

void KeyUtilities::encryptWorldKey( const char* plainWorldKey, const char* accountSubKey, char* encryptedWorldKey )
{
    int plainTextConsumedLen( 0 );
    int cipherTextProducedLen( 0 );
    m_encryptor->setKey( accountSubKey );
    m_encryptor->encrypt( plainWorldKey,
                         encryptedWorldKey,
                         _worldKeySize,
                         encryptedWorldKeySize(),
                         plainTextConsumedLen,
                         cipherTextProducedLen );
}

void KeyUtilities::decryptWorldKey( const char* encryptedWorldKey, const char* accountSubKey, char* plainWorldKey )
{
    int cipherTextConsumedLen( 0 );
    int plainTextProducedLen( 0 );
    m_encryptor->setKey( accountSubKey );
    m_encryptor->decrypt( encryptedWorldKey,
                         plainWorldKey,
                         encryptedWorldKeySize(),
                         _worldKeySize,
                         cipherTextConsumedLen,
                         plainTextProducedLen );
}

int KeyUtilities::accountSubKeySize()
{
    return m_hash.digestSize();
}

void KeyUtilities::getAccountSubKey( const char* accountKey, char* accountSubKey )
{
    char* subKeyPlain = new char[_accountKeySize + 1];
    ::memcpy( subKeyPlain, accountKey, _accountKeySize );
    subKeyPlain[_accountKeySize] = '\x00';
    m_hash.reset();
    m_hash.update( subKeyPlain, _accountKeySize + 1 );
    m_hash.finalise( accountSubKey );
    delete[]( subKeyPlain );
}

void KeyUtilities::getAccountAuthKey( const char* accountKey, char* accountAuthKey )
{
    char* subKeyPlain = new char[_accountKeySize + 1];
    ::memcpy( subKeyPlain, accountKey, _accountKeySize );
    subKeyPlain[_accountKeySize] = '\x01';
    m_hash.reset();
    m_hash.update( subKeyPlain, _accountKeySize + 1 );
    m_hash.finalise( accountAuthKey );
    delete[]( subKeyPlain );
}

int KeyUtilities::deviceAuthKeySize()
{
    return _deviceAuthKeySize;
}

int KeyUtilities::keyHashSize()
{
    return m_hash.digestSize();
}

void KeyUtilities::getAccountAuthKeyHash( const char* accountAuthKey, char* accountAuthKeyHash )
{
    m_hash.reset();
    m_hash.update( accountAuthKey, accountSubKeySize() );
    m_hash.finalise( accountAuthKeyHash );
}

void KeyUtilities::getDeviceAuthKeyHash( const char* deviceAuthKey, char* deviceAuthKeyHash )
{
    m_hash.reset();
    m_hash.update( deviceAuthKey, _deviceAuthKeySize );
    m_hash.finalise( deviceAuthKeyHash );
}

int KeyUtilities::sealingKeySize()
{
    return _sealingKeySize;
}

void KeyUtilities::generateSealingKey( char* sealingKey )
{
    m_entropySource.generate( sealingKey, _sealingKeySize );
}
    
int KeyUtilities::sealedWorldKeySize()
{
    return( _worldKeySize + m_encryptor->overhead() );
}

void KeyUtilities::sealWorldKey( const char* plainWorldKey, const char* sealingKey, char* sealedWorldKey )
{
    int plainTextConsumedLen( 0 );
    int cipherTextProducedLen( 0 );
    m_encryptor->setKey( sealingKey );
    m_encryptor->encrypt( plainWorldKey,
                         sealedWorldKey,
                         _worldKeySize,
                         sealedWorldKeySize(),
                         plainTextConsumedLen,
                         cipherTextProducedLen );
}

void KeyUtilities::unsealWorldKey( const char* sealedWorldKey, const char* sealingKey, char* plainWorldKey )
{
    int cipherTextConsumedLen( 0 );
    int plainTextProducedLen( 0 );
    m_encryptor->setKey( sealingKey );
    m_encryptor->decrypt( sealedWorldKey,
                         plainWorldKey,
                         sealedWorldKeySize(),
                         _worldKeySize,
                         cipherTextConsumedLen,
                         plainTextProducedLen );
}

} // namespace Agape
