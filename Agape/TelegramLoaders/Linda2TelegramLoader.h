#ifndef AGAPE_TELEGRAM_LOADERS_LINDA2_H
#define AGAPE_TELEGRAM_LOADERS_LINDA2_H

#include "Actors/NativeActors/NativeActor.h"
#include "World/Telegram.h"
#include "Collections.h"
#include "Promise.h"
#include "TelegramLoader.h"

using namespace Agape::Linda2;
using namespace Agape::World;

namespace Agape
{

namespace Linda2
{
class Tuple;
class TupleRouter;
} // namespace Linda2

namespace Timers
{
class Factory;
} // namespace Timers

class String;

namespace TelegramLoaders
{

class Linda2 : public TelegramLoader, public Actors::Native
{
public:
    Linda2( const String& recipientSnowflake,
            TupleRouter& tupleRouter,
            Timers::Factory& timerFactory );
    ~Linda2();

    virtual bool load( Vector< Telegram >& telegrams );
    virtual bool loadSent( Vector< Telegram >& telegrams );
    virtual bool send( const Telegram& telegram );
    virtual bool markRead( const Telegram& telegram );
    virtual bool erase( const Telegram& telegram );
    virtual bool unread( Map< String, int >& numUnread, bool allDevices );

    virtual bool accept( Tuple& tuple );

private:
    TupleRouter& m_tupleRouter;
    Timers::Factory& m_timerFactory;

    int m_currentItem;
    int m_totalItems;

    Promise m_telegramLoadResponse;
    Promise m_telegramLoadSentResponse;
    Promise m_telegramSendResponse;
    Promise m_telegramMarkReadResponse;
    Promise m_telegramEraseResponse;
    Promise m_telegramUnreadResponse;

    Vector< Telegram >* m_telegrams;
};

} // namespace TelegramLoaders

} // namespace Agape

#endif // AGAPE_SCENE_LOADERS_LINDA2_H
