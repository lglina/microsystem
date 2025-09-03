#include "World/WorldCoordinates.h"
#include "PresenceLoader.h"

using namespace Agape::World;

namespace Agape
{

bool PresenceLoader::noReceiveRequests( false );

PresenceLoader::PresenceLoader( const Coordinates& coordinates ) :
  m_coordinates( coordinates )
{
}

PresenceLoader::~PresenceLoader()
{
}

} // namespace Agape
