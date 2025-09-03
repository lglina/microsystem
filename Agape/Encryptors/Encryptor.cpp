#include "Encryptor.h"
#include "String.h"

#include "Utils/base64/base64.h"

#include <stdint.h>

//#include <iostream>

namespace Agape
{

String Encryptor::encrypt( const String& plainText, bool base64 )
{
    int plainTextConsumedLen( 0 );
    int cipherTextProducedLen( 0 );
    String cipherText( plainText.size() + overhead(), '\0' );
    encrypt( &plainText[0], &cipherText[0], plainText.size(), cipherText.size(), plainTextConsumedLen, cipherTextProducedLen );

    /*
    std::cerr << "Encrypt: Plaintext length: " << plainText.size()
              << " Ciphertext length: " << cipherText.size()
              << " Produced length: " << cipherTextProducedLen << std::endl;
    */

    if( base64 )
    {
        String encodedCipherText( Base64encode_len( cipherText.size() ), '\0' );
        Base64encode( &encodedCipherText[0],
                      &cipherText[0],
                      cipherText.size() );
        encodedCipherText.resize( encodedCipherText.length() - 1 );

        /*
        std::cerr << "Base64 encode: Input length: " << cipherText.size()
                  << " Encoded length: " << encodedCipherText.size() << std::endl;
        */
        
        return encodedCipherText;
    }
    else
    {
        return cipherText;
    }
}

String Encryptor::decrypt( const String& cipherText, bool base64 )
{
    String decodedCipherText;
    if( base64 )
    {
        decodedCipherText.resize( Base64decode_len( cipherText.c_str() ), '\0' ); // Returns max. bytes required, including NUL terminator.
        decodedCipherText.resize( Base64decode( &decodedCipherText[0],
                                                cipherText.c_str() ) ); // Returns actual bytes decoded, not including NUL.
    }
    else
    {
        decodedCipherText = cipherText;
    }

    int plainTextAllocSize( decodedCipherText.size() - overhead() );
    if( plainTextAllocSize < 0 )
    {
        // Uh oh! Base64 decode failure?
        return String();
    }

    int cipherTextConsumedLen( 0 );
    int plainTextProducedLen( 0 );
    String plainText( plainTextAllocSize, '\0' );
    decrypt( &decodedCipherText[0], &plainText[0], decodedCipherText.size(), plainText.size(), cipherTextConsumedLen, plainTextProducedLen );

    /*
    std::cerr << "Decrypt: Ciphertext length: " << decodedCipherText.size()
              << " Plaintext length: " << plainText.size()
              << " Produced length: " << plainTextProducedLen << std::endl;
    */

    return plainText;
}

} // namespace Agape
