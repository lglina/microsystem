#include "EntropySources/EntropySource.h"
#include "AESEncryptor.h"
#include "String.h"

#include "Encryptors/AES/tiny-AES-c/aes.hpp"

#include <cstdint>
#include <cstring>

#include <string.h>

namespace Agape
{

namespace Encryptors
{

AES::AES( EntropySource& entropySource ) :
  m_entropySource( entropySource )
{
    // If we stuff up and forget to setKey(), use some random
    // key, rather than encrypting with all zeroes or something!
    m_entropySource.generate( (char*)m_key, AES_KEYLEN );
}

void AES::setKey( const char* key )
{
    ::memcpy( m_key, key, AES_KEYLEN );
}

void AES::encrypt( const char* plainText,
                   char* cipherText,
                   int plainTextLen,
                   int cipherTextLen,
                   int& plainTextConsumedLen,
                   int& cipherTextProducedLen )
{
    if( cipherTextLen < ( plainTextLen + AES_BLOCKLEN ) )
    {
        plainTextConsumedLen = 0;
        cipherTextProducedLen = 0;
        return;
    }

    // Generate IV.
    std::uint8_t iv[AES_BLOCKLEN];
    m_entropySource.generate( reinterpret_cast< char* >( &iv ), AES_BLOCKLEN );

    // Init AES.
    struct AES_ctx context;
    AES_init_ctx_iv( &context, m_key, iv );

    // Store IV.
    char* cipherPtr( cipherText );
    std::memcpy( cipherPtr, iv, AES_BLOCKLEN );
    cipherPtr += AES_BLOCKLEN;
    cipherTextProducedLen += AES_BLOCKLEN;

    // Copy plaintext to ciphertext buffer and encrypt.
    // FIXME: We don't *have* to do padding in CTR mode, but we should!?
    // use PKCS#7?
    std::memcpy( cipherPtr, plainText, plainTextLen );
    AES_CTR_xcrypt_buffer( &context, reinterpret_cast< std::uint8_t* >( cipherPtr ), plainTextLen );

    cipherTextProducedLen += plainTextLen;
    plainTextConsumedLen = plainTextLen;
}

void AES::decrypt( const char* cipherText,
                   char* plainText,
                   int cipherTextLen,
                   int plainTextLen,
                   int& cipherTextConsumedLen,
                   int& plainTextProducedLen )
{
    if( plainTextLen < ( cipherTextLen - AES_BLOCKLEN ) )
    {
        cipherTextConsumedLen = 0;
        plainTextProducedLen = 0;
        return;
    }

    // Retrieve IV.
    const char* cipherPtr( cipherText );
    int cipherTextRemain( cipherTextLen );
    std::uint8_t iv[AES_BLOCKLEN];
    std::memcpy( iv, cipherPtr, AES_BLOCKLEN );
    cipherPtr += AES_BLOCKLEN;
    cipherTextRemain -= AES_BLOCKLEN;

    // Init AES.
    struct AES_ctx context;
    AES_init_ctx_iv( &context, m_key, iv );

    // Copy ciphertext to plaintext buffer and decrypt.
    std::memcpy( plainText, cipherPtr, cipherTextRemain );
    AES_CTR_xcrypt_buffer( &context, reinterpret_cast< std::uint8_t* >( plainText ), cipherTextRemain );

    cipherTextConsumedLen = cipherTextLen;
    plainTextProducedLen = cipherTextLen - AES_BLOCKLEN;
}

void AES::status( Agape::String& status )
{
}

void AES::run()
{
}

int AES::keySize()
{
    return AES_BLOCKLEN;
}

int AES::overhead()
{
    return AES_BLOCKLEN;
}

} // namespace Encryptors

} // namespace Agape
