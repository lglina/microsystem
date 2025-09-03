#ifndef AGAPE_UI_DIALOGUE_H
#define AGAPE_UI_DIALOGUE_H

#include "String.h"

namespace Agape
{

class Terminal;
class WindowManager;

namespace UI
{

class Dialogue
{
public:
    enum Type
    {
        normal,
        success,
        error
    };

    Dialogue( WindowManager& windowManager,
              const String& windowName,
              const String& normalBackgroundAssetName,
              const String& successBackgroundAssetName,
              const String& errorBackgroundAssetName );

    static void show( enum Type type );
    static void hide();
    static void drawTitle( const String& title );
    static void drawMessage( const String& message );

private:
    void _show( enum Type type );
    void _hide();
    void _drawTitle( const String& title );
    void _drawMessage( const String& message );

    void drawBackground();

    static Dialogue* s_instance;

    WindowManager& m_windowManager;
    String m_windowName;
    String m_normalBackgroundAssetName;
    String m_successBackgroundAssetName;
    String m_errorBackgroundAssetName;

    Terminal* m_terminal;

    enum Type m_type;
};

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_DIALOGUE_H
