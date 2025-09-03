#ifndef AGAPE_WINDOW_MANAGER_H
#define AGAPE_WINDOW_MANAGER_H

#include "Collections.h"
#include "GraphicsDrivers/GraphicsDriver.h"
#include "Utils/Cartesian.h"
#include "String.h"

namespace Agape
{

class Terminal;

class WindowManager
{
public:
    struct TerminalWindow
    {
        TerminalWindow();

        const GraphicsDriver::Window* m_window;
        Terminal* m_terminal;
    };

    WindowManager( GraphicsDriver& graphicsDriver );

    void createTerminalWindow( const TerminalWindow& terminalWindow );
    bool getTerminalWindow( const String& name, TerminalWindow& terminalWindow );
    void setTerminalWindowVisible( const String& name, bool visible );
    void moveWindow( const String& name, const Point& origin );

private:
    void repaintTopmost( const Rectangle& repaintRect );

    GraphicsDriver& m_graphicsDriver;
    Map< String, TerminalWindow > m_terminalWindows;
};

} // namespace Agape

#endif // AGAPE_WINDOW_MANAGER_H
