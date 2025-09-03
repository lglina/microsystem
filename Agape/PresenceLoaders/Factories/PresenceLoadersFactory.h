#ifndef AGAPE_PRESENCE_LOADERS_FACTORY_H
#define AGAPE_PRESENCE_LOADERS_FACTORY_H

#include "PresenceLoaders/PresenceLoader.h"

namespace Agape
{

namespace World
{
class Coordinates;
} // namespace World

namespace PresenceLoaders
{

class Factory
{
public:
    virtual ~Factory() {}
    
    virtual PresenceLoader* makeLoader( const Coordinates& coordinates, bool receiveRequests = true ) = 0;
};

} // namespace PresenceLoaders

} // namespace Agape

#endif // AGAPE_PRESENCE_LOADERS_FACTORY_H
