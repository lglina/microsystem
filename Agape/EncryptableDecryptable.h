#ifndef AGAPE_ENCRYPTABLE_DECRYPTABLE_H
#define AGAPE_ENCRYPTABLE_DECRYPTABLE_H

#include "String.h"

namespace Agape
{

class Encryptor;

class EncryptableDecryptable
{
public:
    virtual bool encrypt( Encryptor& encryptor ) = 0;
    virtual bool decrypt( Encryptor& encryptor ) = 0;
};

} // namespace Agape

#endif // AGAPE_ENCRYPTABLE_DECRYPTABLE_H
