#include "TelegramLoaders/NullTelegramLoader.h"
#include "TelegramLoaders/TelegramLoader.h"
#include "NullTelegramLoaderFactory.h"
#include "String.h"

namespace Agape
{

using namespace World;

namespace TelegramLoaders
{

namespace Factories
{

Null::Null()
{
}

TelegramLoader* Null::makeLoader( const String& recipientSnowflake )
{
    return new TelegramLoaders::Null( recipientSnowflake );
}

} // namespace Factories

} // namespace TelegramLoaders

} // namespace Agape
