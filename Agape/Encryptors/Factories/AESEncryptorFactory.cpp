#include "Encryptors/AES/AESEncryptor.h"
#include "Encryptors/Encryptor.h"
#include "EntropySources/EntropySource.h"
#include "AESEncryptorFactory.h"

namespace Agape
{

namespace Encryptors
{

namespace Factories
{

AES::AES( EntropySource& entropySource ) :
  m_entropySource( entropySource )
{
}

Encryptor* AES::makeEncryptor()
{
    return new Encryptors::AES( m_entropySource );
}

} // namespace Factories

} // namespace Encryptors

} // namespace Agape
