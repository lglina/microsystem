#ifndef AGAPE_ENCRYPTORS_FACTORY_H
#define AGAPE_ENCRYPTORS_FACTORY_H

namespace Agape
{

class Encryptor;

namespace Encryptors
{

class Factory
{
public:
    virtual ~Factory() {}
    
    virtual Encryptor* makeEncryptor() = 0;
};

} // namespace Encryptors

} // namespace Agape

#endif // AGAPE_ENCRYPTORS_FACTORY_H
