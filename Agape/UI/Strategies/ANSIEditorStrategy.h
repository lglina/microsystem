#ifndef AGAPE_STRATEGIES_ANSI_EDITOR_H
#define AGAPE_STRATEGIES_ANSI_EDITOR_H

#include "UI/Forms/Form.h"
#include "UI/Strategy.h"
#include "ANSIEditorFactory.h"
#include "String.h"

namespace Agape
{

namespace ANSIEditor
{
class ANSIEditor;
class Factory;
} // namespace ANSIEditor

namespace AssetLoaders
{
class Factory;
} // namespace AssetLoaders

namespace Timers
{
class Factory;
} // namespace Timers

namespace World
{
class Compositor;
class Coordinates;
class SceneItem;
} // namespace World

class InputDevice;
class Terminal;
class Timer;
class WindowManager;

namespace UI
{

class Dialogue;
class Hotkeys;

namespace Strategies
{

class ANSIEditor : public Strategy
{
public:
    ANSIEditor( InputDevice& inputDevice,
                Compositor& compositor,
                Agape::ANSIEditor::Factory& ansiEditorFactory,
                AssetLoaders::Factory& assetLoaderFactory,
                AssetLoaders::Factory& programAssetLoaderFactory,
                const World::Coordinates& coordinates,
                Dialogue& dialogue,
                Hotkeys& hotkeys,
                WindowManager& windowManager,
                const String& formWindowName,
                Timers::Factory& timerFactory );
    ~ANSIEditor();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        none,
        selectObject,
        selectAsset,
        editing,
        selectTemplate,
        selectTemplateSuccess,
        selectTemplateError,
        saveAs,
        quickSaveSuccess,
        discard,
        saveError
    };

    bool openEditor();
    bool quickSave();
    bool closeEditor( bool save, const String& as );
    bool closeEditor( bool save );
    void afterClose();
    
    void drawSelectAsset();
    void drawSelectTemplate();
    void drawSelectTemplateSuccess();
    void drawSelectTemplateError();
    void drawSaveAs();
    void drawQuickSaveSuccess();
    void drawDiscard();
    void drawSaveError();
    void hideDialogue();
    void hideFormWindow();

    void drawHotkeysSelectObject();
    void drawHotkeysEditing();
    void drawHotkeysEditingHeights();

    void closeForm();

    void setAssetName();
    bool setTemplate();

    InputDevice& m_inputDevice;
    Compositor& m_compositor;
    Agape::ANSIEditor::Factory& m_ansiEditorFactory;
    AssetLoaders::Factory& m_assetLoaderFactory;
    AssetLoaders::Factory& m_programAssetLoaderFactory;
    const World::Coordinates& m_coordinates;
    Dialogue& m_dialogue;
    Hotkeys& m_hotkeys;
    WindowManager m_windowManager;
    String m_formWindowName;

    Timer* m_timer;

    bool m_completed;

    enum State m_state;

    const World::SceneItem* m_sceneItem;

    String m_assetName;

    Forms::Form* m_currentForm;

    Terminal* m_formTerminal;
    Agape::ANSIEditor::ANSIEditor* m_ansiEditor;

    int m_prevFGColour;
    int m_prevBGColour;

    bool m_doInsert;

    bool m_editingHeights;

    bool m_needInvalidate;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_ANSI_EDITOR_H
