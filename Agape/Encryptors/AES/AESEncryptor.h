#ifndef AGAPE_ENCRYPTORS_AES_H
#define AGAPE_ENCRYPTORS_AES_H

#include "Encryptors/Encryptor.h"
#include "String.h"

#include "Encryptors/AES/tiny-AES-c/aes.hpp"

#include <cstdint>

namespace Agape
{

class EntropySource;

namespace Encryptors
{

class AES : public Encryptor
{
public:
    AES( EntropySource& entropySource );

    virtual void setKey( const char* key );

    virtual void encrypt( const char* plainText,
                          char* cipherText,
                          int plainTextLen,
                          int cipherTextLen,
                          int& plainTextConsumedLen,
                          int& cipherTextProducedLen );

    virtual void decrypt( const char* cipherText,
                          char* plainText,
                          int cipherTextLen,
                          int plainTextLen,
                          int& cipherTextConsumedLen,
                          int& plainTextProducedLen );

    virtual void status( Agape::String& status );

    virtual void run();

    virtual int keySize();
    virtual int overhead();

private:
    EntropySource& m_entropySource;
    std::uint8_t m_key[AES_KEYLEN];
};

} // namespace Encryptors

} // namespace Agape

#endif // AGAPE_ENCRYPTORS_AES_H
