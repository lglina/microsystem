#ifndef AGAPE_UI_PRESENCE_H
#define AGAPE_UI_PRESENCE_H

#include "World/WorldCoordinates.h"

namespace Agape
{

namespace PresenceLoaders
{
class Factory;
} // namespace PresenceLoaders

namespace World
{
class Metadata;
} // namespace World

class Clock;
class PresenceLoader;
class String;
class Terminal;
class WindowManager;

using namespace World;

namespace UI
{

class Presence
{
public:
    Presence( WindowManager& windowManager,
              const String& windowName,
              Coordinates& coordinates,
              Metadata& metadata,
              PresenceLoaders::Factory& presenceLoaderFactory,
              Clock& clock );
    ~Presence();

    void draw();
    void clear();

private:
    Coordinates& m_coordinates;
    Metadata& m_metadata;
    PresenceLoaders::Factory& m_presenceLoaderFactory;
    Clock& m_clock;

    Terminal* m_terminal;

    Coordinates m_currentCoordinates;
    PresenceLoader* m_presenceLoader;
};

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_PRESENCE_H
