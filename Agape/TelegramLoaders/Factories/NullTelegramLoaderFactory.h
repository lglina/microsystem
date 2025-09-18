#ifndef AGAPE_TELEGRAM_LOADERS_FACTORIES_NULL_H
#define AGAPE_TELEGRAM_LOADERS_FACTORIES_NULL_H

#include "String.h"
#include "TelegramLoadersFactory.h"

namespace Agape
{

namespace World
{
class Metadata;
} // namespace World

using namespace World;

class TelegramLoader;

namespace TelegramLoaders
{

namespace Factories
{

class Null : public Factory
{
public:
    Null();

    virtual TelegramLoader* makeLoader( const String& recipientSnowflake );
};

} // namespace Factories

} // namespace TelegramLoaders

} // namespace Agape

#endif // AGAPE_TELEGRAM_LOADERS_FACTORIES_NULL_H
