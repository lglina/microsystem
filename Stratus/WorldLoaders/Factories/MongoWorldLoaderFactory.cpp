#include "WorldLoaders/MongoWorldLoader.h"
#include "Authenticator.h"
#include "MongoWorldLoaderFactory.h"
#include "TupleRouter.h"

namespace Agape
{

namespace WorldLoaders
{

namespace Factories
{

Mongo::Mongo( Stratus::Authenticator& authenticator ) :
  m_authenticator( authenticator )
{
}

WorldLoader* Mongo::makeLoader()
{
    return new WorldLoaders::Mongo( m_authenticator );
}

} // namespace Factories

} // namespace WorldLoaders

} // namespace Agape
