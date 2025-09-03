#ifndef AGAPE_TELEGRAM_LOADERS_LINDA2_RESPONDER_H
#define AGAPE_TELEGRAM_LOADERS_LINDA2_RESPONDER_H

#include "Actors/NativeActors/NativeActor.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace Linda2
{
class Tuple;
class TupleRouter;
} // namespace Linda2

namespace TelegramLoaders
{

class Factory;

class Linda2Responder : public Actors::Native
{
public:
    Linda2Responder( TupleRouter& tupleRouter, TelegramLoaders::Factory& telegramLoaderFactory );
    virtual ~Linda2Responder();

    virtual bool accept( Tuple& tuple );

    void reset();

private:
    void load( const Tuple& tuple );
    void loadSent( const Tuple& tuple );
    void send( const Tuple& tuple );
    void markRead( const Tuple& tuple );
    void erase( const Tuple& tuple );
    void unread( const Tuple& tuple );

    TupleRouter& m_tupleRouter;
    TelegramLoaders::Factory& m_telegramLoaderFactory;
};

} // namespace TelegramLoaders

} // namespace Agape

#endif // AGAPE_TELEGRAM_LOADERS_LINDA2_RESPONDER_H
