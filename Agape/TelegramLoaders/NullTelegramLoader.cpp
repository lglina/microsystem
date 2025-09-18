#include "World/Telegram.h"
#include "Collections.h"
#include "FileWriter.h"
#include "NullTelegramLoader.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"

using namespace Agape::World;

namespace Agape
{

namespace TelegramLoaders
{

Null::Null( const String& recipientSnowflake ) :
  TelegramLoader( recipientSnowflake )
{
}

bool Null::load( Vector< Telegram >& telegrams )
{
    return true;
}

bool Null::loadSent( Vector< Telegram >& telegrams )
{
    return true;
}

bool Null::send( const Telegram& telegram )
{
    return true;
}

bool Null::markRead( const Telegram& telegram )
{
    return true;
}

bool Null::erase( const Telegram& telegram )
{
    return true;
}

bool Null::unread( Map< String, int >& numUnread, bool allDevices )
{
    return true;
}

} // namespace TelegramLoaders

} // namespace Agape
