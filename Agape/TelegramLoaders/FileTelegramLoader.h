#ifndef AGAPE_TELEGRAM_LOADERS_FILE_H
#define AGAPE_TELEGRAM_LOADERS_FILE_H

#include "World/Telegram.h"
#include "Collections.h"
#include "String.h"
#include "TelegramLoader.h"

using namespace Agape::World;

namespace Agape
{

namespace TelegramLoaders
{

class File : public TelegramLoader
{
public:
    File( const String& recipientSnowflake,
          const String& filename,
          const String& worldID );

    virtual bool load( Vector< Telegram >& telegrams );
    virtual bool loadSent( Vector< Telegram >& telegrams );
    virtual bool send( const Telegram& telegram );
    virtual bool markRead( const Telegram& telegram );
    virtual bool erase( const Telegram& telegram );
    virtual bool unread( Map< String, int >& numUnread, bool allDevices );

private:
    String m_filename;
    String m_worldID;
};

} // namespace TelegramLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_FILE_H
