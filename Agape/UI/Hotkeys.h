#ifndef AGAPE_UI_HOTKEYS_H
#define AGAPE_UI_HOTKEYS_H

namespace Agape
{

class String;
class Terminal;
class WindowManager;

namespace UI
{

class Hotkeys
{
public:
    Hotkeys( WindowManager& windowManager,
             const String& windowName );

    void clear();
    void show( const String& keys, const String& name );
    void spacer();

private:
    Terminal* m_terminal;
};

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_HOTKEYS_H
