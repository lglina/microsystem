#ifndef AGAPE_TELEGRAM_LOADERS_FACTORIES_LINDA2_H
#define AGAPE_TELEGRAM_LOADERS_FACTORIES_LINDA2_H

#include "TelegramLoadersFactory.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace Linda2
{
class TupleRouter;
} // namespace Linda2

namespace Timers
{
class Factory;
} // namespace Timers

class String;
class TelegramLoader;

namespace TelegramLoaders
{

namespace Factories
{

class Linda2 : public Factory
{
public:
    Linda2( TupleRouter& tupleRouter, Timers::Factory& timerFactory );

    virtual TelegramLoader* makeLoader( const String& recipientSnowflake );

private:
    TupleRouter& m_tupleRouter;
    Timers::Factory& m_timerFactory;
};

} // namespace Factories

} // namespace TelegramLoaders

} // namespace Agape

#endif // AGAPE_TELEGRAM_LOADERS_FACTORIES_LINDA2_H
