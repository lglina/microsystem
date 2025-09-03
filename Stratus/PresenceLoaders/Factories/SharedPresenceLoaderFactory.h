#ifndef AGAPE_PRESENCE_LOADERS_FACTORIES_SHARED_H
#define AGAPE_PRESENCE_LOADERS_FACTORIES_SHARED_H

#include "PresenceLoadersFactory.h"

namespace Agape
{

namespace Stratus
{
class Authenticator;
} // namespace Stratus

namespace World
{
class Coordinates;
} // namespace World

class Clock;
class PresenceLoader;

using namespace Stratus;

namespace PresenceLoaders
{

class SharedPresenceStore;

namespace Factories
{

class Shared : public Factory
{
public:
    Shared( SharedPresenceStore& sharedPresenceStore,
            Clock& clock,
            Authenticator& authenticator );

    virtual PresenceLoader* makeLoader( const World::Coordinates& coordinates, bool receiveRequests );

private:
    SharedPresenceStore& m_sharedPresenceStore;
    Clock& m_clock;
    Authenticator& m_authenticator;
};

} // namespace Factories

} // namespace PresenceLoaders

} // namespace Agape

#endif // AGAPE_PRESENCE_LOADERS_FACTORIES_SHARED_H
