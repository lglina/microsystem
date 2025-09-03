#ifndef AGAPE_ENCRYPTORS_FACTORIES_AES_H
#define AGAPE_ENCRYPTORS_FACTORIES_AES_H

#include "EncryptorsFactory.h"

namespace Agape
{

class Encryptor;
class EntropySource;

namespace Encryptors
{

namespace Factories
{

class AES : public Factory
{
public:
    AES( EntropySource& entropySource );
    
    virtual Encryptor* makeEncryptor();

private:
    EntropySource& m_entropySource;
};

} // namespace Factories

} // namespace Encryptors

} // namespace Agape

#endif // AGAPE_ENCRYPTORS_FACTORIES_AES_H
