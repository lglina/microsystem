#include "TelegramLoaders/Linda2TelegramLoader.h"
#include "TelegramLoaders/TelegramLoader.h"
#include "Timers/Factories/TimerFactory.h"
#include "Linda2TelegramLoaderFactory.h"
#include "String.h"
#include "TupleRouter.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace TelegramLoaders
{

namespace Factories
{

Linda2::Linda2( TupleRouter& tupleRouter, Timers::Factory& timerFactory ) :
  m_tupleRouter( tupleRouter ),
  m_timerFactory( timerFactory )
{
}

TelegramLoader* Linda2::makeLoader( const String& recipientSnowflake )
{
    return new TelegramLoaders::Linda2( recipientSnowflake, m_tupleRouter, m_timerFactory );
}

} // namespace Factories

} // namespace TelegramLoaders

} // namespace Agape
