#ifndef AGAPE_TELEGRAM_LOADERS_FACTORY_H
#define AGAPE_TELEGRAM_LOADERS_FACTORY_H

namespace Agape
{

class String;
class TelegramLoader;

namespace TelegramLoaders
{

class Factory
{
public:
    virtual ~Factory() {}
    
    virtual TelegramLoader* makeLoader( const String& recipientSnowflake ) = 0;
};

} // namespace TelegramLoaders

} // namespace Agape

#endif // AGAPE_TELEGRAM_LOADERS_FACTORY_H
