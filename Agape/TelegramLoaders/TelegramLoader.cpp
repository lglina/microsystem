#include "World/Telegram.h"
#include "Collections.h"
#include "String.h"
#include "TelegramLoader.h"

using namespace Agape::World;

namespace Agape
{

TelegramLoader::TelegramLoader( const String& recipientSnowflake ) :
  m_recipientSnowflake( recipientSnowflake )
{
}

TelegramLoader::~TelegramLoader()
{
}

} // namespace Agape
