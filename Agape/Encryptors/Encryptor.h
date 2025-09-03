#ifndef AGAPE_ENCRYPTOR_H
#define AGAPE_ENCRYPTOR_H

#include "Runnable.h"
#include "String.h"

namespace Agape
{

class Encryptor : public Runnable
{
public:
    virtual ~Encryptor() {}
    
    virtual void setKey( const char* key ) = 0;

    virtual void encrypt( const char* plainText,
                          char* cipherText,
                          int plainTextLen,
                          int cipherTextLen,
                          int& plainTextConsumedLen,
                          int& cipherTextProducedLen ) = 0;

    virtual void decrypt( const char* cipherText,
                          char* plainText,
                          int cipherTextLen,
                          int plainTextLen,
                          int& cipherTextConsumedLen,
                          int& plainTextProducedLen ) = 0;

    String encrypt( const String& plainText, bool base64 = true );
    String decrypt( const String& cipherText, bool base64 = true );

    virtual void status( Agape::String& status ) = 0;

    virtual void run() = 0;

    virtual int keySize() = 0;
    virtual int overhead() = 0;
};

} // namespace Agape

#endif // AGAPE_ENCRYPTOR_H
