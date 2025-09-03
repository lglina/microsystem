#ifndef AGAPE_ENCRYPTORS_FACTORIES_AES_BLOCK_H
#define AGAPE_ENCRYPTORS_FACTORIES_AES_BLOCK_H

#include "BlockEncryptorsFactory.h"

namespace Agape
{

class BlockEncryptor;
class EntropySource;

namespace Encryptors
{

namespace Factories
{

class AESBlock : public BlockFactory
{
public:
    AESBlock( EntropySource& entropySource );
    
    virtual BlockEncryptor* makeEncryptor();

private:
    EntropySource& m_entropySource;
};

} // namespace Factories

} // namespace Encryptors

} // namespace Agape

#endif // AGAPE_ENCRYPTORS_FACTORIES_AES_BLOCK_H
