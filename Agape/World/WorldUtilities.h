#ifndef AGAPE_WORLD_UTILITIES_H
#define AGAPE_WORLD_UTILITIES_H

namespace Agape
{

namespace Encryptors
{
class Factory;
} // namespace Encryptors

class Encryptor;
class EntropySource;
class Hash;
class KeyUtilities;
class Snowflake;
class String;

namespace World
{

class Metadata;
class User;

class Utilities
{
public:
    Utilities( KeyUtilities& keyUtilities,
               Snowflake& snowflake,
               Encryptors::Factory& encryptorFactory );
    ~Utilities();

    void generateWorldKey( char* worldKey );
    int worldKeySize();

    void generateCreateMetadata( const char* worldKey,
                                 const char* accountKey,
                                 const String& worldName,
                                 const String& userName,
                                 int userGlyphBase,
                                 int userAttributes,
                                 Metadata& plainMetadata,
                                 User& plainUser );

    void generateJoinMetadata( const char* worldKey,
                               const char* accountKey,
                               const String& userName,
                               int userGlyphBase,
                               int userAttributes,
                               Metadata& plainMetadata,
                               User& plainUser );
    

    void encryptMetadata( const char* worldKey,
                          const Metadata& plainMetadata,
                          Metadata& encryptedMetadata );

    void decryptMetadata( const char* worldKey,
                          const Metadata& encryptedMetadata,
                          Metadata& plainMetadata );
    
    void decryptPrivateKey( const Metadata& metadata,
                            const char* accountSubKey,
                            char* worldKey );

private:
    KeyUtilities& m_keyUtilities;
    Snowflake& m_snowflake;
    
    Encryptor* m_encryptor;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_WORLD_UTILITIES_H
