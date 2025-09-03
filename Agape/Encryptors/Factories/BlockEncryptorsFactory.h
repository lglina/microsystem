#ifndef AGAPE_ENCRYPTORS_BLOCK_FACTORY_H
#define AGAPE_ENCRYPTORS_BLOCK_FACTORY_H

namespace Agape
{

class BlockEncryptor;

namespace Encryptors
{

class BlockFactory
{
public:
    virtual ~BlockFactory() {}
    
    virtual BlockEncryptor* makeEncryptor() = 0;
};

} // namespace Encryptors

} // namespace Agape

#endif // AGAPE_ENCRYPTORS_BLOCK_FACTORY_H
