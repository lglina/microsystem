#ifndef AGAPE_UI_TAB_BAR_H
#define AGAPE_UI_TAB_BAR_H

#include "Collections.h"
#include "String.h"
#include "Terminal.h"
#include "WindowManager.h"

namespace Agape
{

namespace UI
{

class TabBar
{
public:
    enum Alignment
    {
        left,
        right
    };

    TabBar( WindowManager& windowManager,
            const String& windowName );

    void create( const String& name,
                 int reservedWidth,
                 enum Alignment alignment,
                 bool visible,
                 const String& text,
                 char attributes );
    void remove( const String& name );

    bool haveTab( const String& name );

    void update( const String& name,
                 const String& text,
                 char attributes = 0 );

    void setVisible( const String& name,
                     bool visible );

    void redrawAll();

private:
    struct Tab
    {
        String m_name;
        int m_reservedWidth;
        enum Alignment m_alignment;
        bool m_visible;
        String m_text;
        char m_attributes;
    };

    void redrawOne( const String& name );
    void doRedraw( const struct Tab& tab, int col );

    Terminal* m_terminal;

    Vector< struct Tab > m_tabs;
};

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_TAB_BAR_H
