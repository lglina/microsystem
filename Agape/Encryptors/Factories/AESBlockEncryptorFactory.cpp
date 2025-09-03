#include "Encryptors/AES/AESBlockEncryptor.h"
#include "Encryptors/BlockEncryptor.h"
#include "EntropySources/EntropySource.h"
#include "AESBlockEncryptorFactory.h"

namespace Agape
{

namespace Encryptors
{

namespace Factories
{

AESBlock::AESBlock( EntropySource& entropySource ) :
  m_entropySource( entropySource )
{
}

BlockEncryptor* AESBlock::makeEncryptor()
{
    return new Encryptors::AESBlock( m_entropySource );
}

} // namespace Factories

} // namespace Encryptors

} // namespace Agape
