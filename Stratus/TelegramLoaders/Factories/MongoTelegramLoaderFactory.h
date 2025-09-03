#ifndef AGAPE_TELEGRAM_LOADERS_FACTORIES_MONGO_H
#define AGAPE_TELEGRAM_LOADERS_FACTORIES_MONGO_H

#include "TelegramLoadersFactory.h"

namespace Agape
{

namespace Stratus
{
class Authenticator;
} // namespace Stratus

class String;
class TelegramLoader;

namespace TelegramLoaders
{

namespace Factories
{

class Mongo : public Factory
{
public:
    Mongo( Authenticator& authenticator );

    virtual TelegramLoader* makeLoader( const String& recipientSnowflake );

private:
    Authenticator& m_authenticator;
};

} // namespace Factories

} // namespace TelegramLoaders

} // namespace Agape

#endif // AGAPE_TELEGRAM_LOADERS_FACTORIES_MONGO_H
