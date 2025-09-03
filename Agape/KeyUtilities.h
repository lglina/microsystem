#ifndef AGAPE_KEY_UTILITIES_H
#define AGAPE_KEY_UTILITIES_H

namespace Agape
{

namespace Encryptors
{
class Factory;
} // namespace Encryptors

class Encryptor;
class EntropySource;
class Hash;

class KeyUtilities
{
public:
    KeyUtilities( EntropySource& entropySource,
                  Encryptors::Factory& encryptorFactory,
                  Hash& hash );
    ~KeyUtilities();

    int worldKeySize();
    void generateWorldKey( char* worldKey );
    void generateItemKey( char* itemKey );

    int worldIDSize();
    void getWorldID( const char* worldKey, char* worldID );
    void getWorldAuthKey( const char* worldKey, char* worldAuthKey );

    int encryptedWorldKeySize();
    void encryptWorldKey( const char* plainWorldKey, const char* accountSubKey, char* encryptedWorldKey );
    void decryptWorldKey( const char* encryptedWorldKey, const char* accountSubKey, char* plainWorldKey );

    int accountSubKeySize();
    void getAccountSubKey( const char* accountKey, char* accountSubKey );
    void getAccountAuthKey( const char* accountKey, char* accountAuthKey );

    int deviceAuthKeySize();

    int keyHashSize();
    void getAccountAuthKeyHash( const char* accountAuthKey, char* accountAuthKeyHash );
    void getDeviceAuthKeyHash( const char* deviceAuthKey, char* deviceAuthKeyHash );

    int sealingKeySize();
    void generateSealingKey( char* sealingKey );
    
    int sealedWorldKeySize();
    void sealWorldKey( const char* plainWorldKey, const char* sealingKey, char* sealedWorldKey );
    void unsealWorldKey( const char* sealedWorldKey, const char* sealingKey, char* plainWorldKey );

private:
    EntropySource& m_entropySource;
    Hash& m_hash;

    Encryptor* m_encryptor;
};

} // namespace Agape

#endif // AGAPE_KEY_UTILITIES_H
