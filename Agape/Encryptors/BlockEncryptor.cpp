#include "BlockEncryptor.h"

namespace Agape
{

BlockEncryptor::BlockEncryptor() :
  m_backingDevice( nullptr ),
  m_openMode( modeNone )
{
}

} // namespace Agape
