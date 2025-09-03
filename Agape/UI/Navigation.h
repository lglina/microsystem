#ifndef AGAPE_UI_NAVIGATION_H
#define AGAPE_UI_NAVIGATION_H

namespace Agape
{

namespace World
{
class Coordinates;
class Metadata;
} // namespace World

class String;
class Terminal;
class WindowManager;

using namespace World;

namespace UI
{

class Navigation
{
public:
    Navigation( WindowManager& windowManager,
                const String& windowName,
                const Coordinates& coordinates,
                const Metadata& worldMetadata );

    void draw();
    void draw( const Coordinates& coordinates );
    void clear();

private:
    const Coordinates& m_coordinates;
    const Metadata& m_worldMetadata;

    Terminal* m_terminal;
};

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_NAVIGATION_H
