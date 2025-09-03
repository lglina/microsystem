#include "TelegramLoaders/MongoTelegramLoader.h"
#include "TelegramLoaders/TelegramLoader.h"
#include "Authenticator.h"
#include "MongoTelegramLoaderFactory.h"
#include "String.h"

namespace Agape
{

using namespace Stratus;

namespace TelegramLoaders
{

namespace Factories
{

Mongo::Mongo( Authenticator& authenticator ) :
  m_authenticator( authenticator )
{
}

TelegramLoader* Mongo::makeLoader( const String& recipientSnowflake )
{
    return new TelegramLoaders::Mongo( recipientSnowflake, m_authenticator );
}

} // namespace Factories

} // namespace TelegramLoaders

} // namespace Agape
