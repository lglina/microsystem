#ifndef AGAPE_WORLD_LOADERS_FACTORIES_MONGO_H
#define AGAPE_WORLD_LOADERS_FACTORIES_MONGO_H

#include "WorldLoadersFactory.h"

namespace Agape
{

namespace Stratus
{
class Authenticator;
} // namespace Stratus

namespace WorldLoaders
{

namespace Factories
{

class Mongo : public Factory
{
public:
    Mongo( Stratus::Authenticator& authenticator );

    virtual WorldLoader* makeLoader();

private:
    Stratus::Authenticator& m_authenticator;
};

} // namespace Factories

} // namespace WorldLoaders

} // namespace Agape

#endif // AGAPE_WORLD_LOADERS_FACTORIES_MONGO_H
