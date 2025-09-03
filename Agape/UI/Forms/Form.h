#ifndef AGAPE_UI_FORMS_FORM_H
#define AGAPE_UI_FORMS_FORM_H

#include "AssetLoaders/AssetLoader.h"
#include "Collections.h"
#include "FormTypes.h"
#include "String.h"
#include "WindowManager.h"

namespace Agape
{

namespace UI
{

namespace Forms
{

class Form
{
public:
    Form( WindowManager& windowManager,
          const String& windowName,
          const String& uiAssetName,
          const Vector< struct Field >& fields,
          Title title );
    ~Form();

    bool openUI();
    bool uiIsOpen() const;

    void setTitle( const Title& title );

    void reset();
    void draw();
    void close();

    void consumeChar( char c );

    void setFieldContents( const String& fieldName, const String& contents );
    String getFieldContents( const String& fieldName ) const;
    
    int getSelectIndex( const String& fieldName ) const;
    void setSelectIndex( const String& fieldName, int index );
    
    const Field& currentField() const;
    bool setCurrentField( const String& fieldName );

    void nextField();
    void prevField();

    void nextSelect();
    void prevSelect();

private:
    void clearField( Vector< struct Field >::iterator& field );
    void drawTitle();
    void drawLabel( Vector< struct Field >::iterator& field );
    void drawText( Vector< struct Field >::iterator& field );
    void drawSelect( Vector< struct Field >::iterator& field, bool focused );
    void drawButtonActive( Vector< struct Field >::iterator& field );
    void positionTextCursor();
    void switchField( Vector< struct Field >::iterator& current, const Vector< struct Field >::iterator& next );
    int textOffsetAtCursor();
    void textAdd( char c );
    void textMoveCursor( char c );
    void textBackspace();

    WindowManager& m_windowManager;
    const String m_windowName;
    const String m_uiAssetName;
    Vector< struct Field > m_fields;
    Vector< struct Field >::iterator m_currentFieldIter;
    Title m_title;

    WindowManager::TerminalWindow m_terminalWindow;
    AssetLoader* m_uiAssetLoader;
    bool m_uiOpen;
    bool m_interactive;
};

} // namespace Forms

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_FORMS_FORM_H
