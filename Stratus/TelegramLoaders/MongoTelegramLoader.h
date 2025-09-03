#ifndef AGAPE_TELEGRAM_LOADERS_MONGO_H
#define AGAPE_TELEGRAM_LOADERS_MONGO_H

#include "World/Telegram.h"
#include "Collections.h"
#include "TelegramLoader.h"

using namespace Agape::World;

namespace Agape
{

namespace Stratus
{
class Authenticator;
} // namespace Stratus

using namespace Stratus;

namespace TelegramLoaders
{

class Mongo : public TelegramLoader
{
public:
    Mongo( const String& recipientSnowflake,
           Authenticator& authenticator );

    virtual bool load( Vector< Telegram >& telegrams );
    virtual bool loadSent( Vector< Telegram >& telegrams );
    virtual bool send( const Telegram& telegram );
    virtual bool markRead( const Telegram& telegram );
    virtual bool erase( const Telegram& telegram );
    virtual bool unread( Map< String, int >& numUnread, bool allDevices );

private:
    Authenticator& m_authenticator;
};

} // namespace TelegramLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_MONGO_H
