#ifndef AGAPE_STRATEGIES_CARLO_H
#define AGAPE_STRATEGIES_CARLO_H

#include "UI/Forms/Form.h"
#include "UI/Forms/FormTypes.h"
#include "UI/Strategy.h"
#include "Collections.h"
#include "EditorFactory.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{
class Factory;
} // namespace AssetLoaders

namespace Editor
{
class Editor;
class Factory;
} // namespace Editor

namespace Carlo
{
class ProgramManager;
} // namespace Carlo

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

class Carlo : public Strategy
{
public:
    Carlo( InputDevice& inputDevice,
           World::Compositor& compositor,
           Editor::Factory& editorFactory,
           const World::Coordinates& coordinates,
           Agape::Carlo::ProgramManager& programManager,
           AssetLoaders::Factory& assetLoaderFactory,
           Dialogue& dialogue,
           Hotkeys& hotkeys,
           WindowManager& windowManager,
           const String& errorsWindowName,
           const String& formWindowName,
           Timers::Factory& timerFactory );
    ~Carlo();

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
        selectProgram,
        selectProgramError,
        editProgram,
        saveAs,
        quickSaveSuccess,
        discard,
        saveError
    };

    bool openEditor();
    bool openTemplateEditor( bool withItem );
    bool openSceneProgEditor();
    bool openWorldProgEditor();
    bool quickSave();
    bool closeEditor( bool save, const String& as );
    bool closeEditor( bool save );
    void afterClose();

    void drawSelectProgram();
    void drawSelectProgramError();
    void drawSaveAs();
    void drawQuickSaveSuccess();
    void drawDiscard();
    void drawSaveError();
    void hideDialogue();
    void hideFormWindow();

    void drawHotkeysSelectObject();

    void closeForm();

    InputDevice& m_inputDevice;
    World::Compositor& m_compositor;
    Editor::Factory& m_editorFactory;
    const World::Coordinates& m_coordinates;
    Agape::Carlo::ProgramManager& m_programManager;
    AssetLoaders::Factory& m_assetLoaderFactory;
    Dialogue& m_dialogue;
    Hotkeys& m_hotkeys;
    WindowManager& m_windowManager;
    String m_errorsWindowName;
    String m_formWindowName;

    Timer* m_timer;

    bool m_completed;

    enum State m_state;
    
    const World::SceneItem* m_sceneItem;
    String m_templateName;
    bool m_editingLinked;

    String m_programName;
    String m_instanceName;
    String m_displayName;
    String m_linkedItemName;

    Editor::Editor* m_editor;

    Terminal* m_formTerminal;
    Forms::Form* m_currentForm;

    bool m_needInvalidate;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_STRATEGIES_CARLO_H
