#ifndef AGAPE_TELEGRAM_LOADER_H
#define AGAPE_TELEGRAM_LOADER_H

#include "World/Telegram.h"
#include "Collections.h"
#include "String.h"

using namespace Agape::World;

namespace Agape
{

class TelegramLoader
{
public:
    TelegramLoader( const String& recipientSnowflake );
    virtual ~TelegramLoader();

    virtual bool load( Vector< Telegram >& telegrams ) = 0;
    virtual bool loadSent( Vector< Telegram >& telegrams ) = 0;
    virtual bool send( const Telegram& telegram ) = 0;
    virtual bool markRead( const Telegram& telegram ) = 0;
    virtual bool erase( const Telegram& telegram ) = 0;
    virtual bool unread( Map< String, int >& numUnread, bool allDevices ) = 0;

protected:
    String m_recipientSnowflake;
};

} // namespace Agape

#endif // AGAPE_SCENE_LOADER_H
