#ifndef AGAPE_UI_STRATEGIES_EDIT_WORLD_H
#define AGAPE_UI_STRATEGIES_EDIT_WORLD_H

#include "UI/Strategy.h"
#include "World/SceneItem.h"
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

namespace Timers
{
class Factory;
} // namespace Timers

namespace World
{
    class Compositor;
    class Coordinates;
    class Editor;
    class Metadata;
    class SceneItem;
}

class InputDevice;
class Terminal;
class Timer;
class Value;
class WindowManager;

using namespace World;

namespace UI
{

namespace Forms
{
class Form;
} // namespace Forms

class Dialogue;
class Hotkeys;

namespace Strategies
{

class EditWorld : public Strategy
{
public:
    EditWorld( InputDevice& inputDevice,
               WindowManager& windowManager,
               const String& mapWindowName,
               const String& formWindowName,
               Dialogue& dialogue,
               Hotkeys& hotkeys,
               Timers::Factory& timerFactory,
               AssetLoaders::Factory& assetLoaderFactory,
               Agape::Editor::Factory& editorFactory,
               const Metadata& worldMetadata,
               const Coordinates& coordinates,
               Compositor& compositor );
    ~EditWorld();

    virtual void enter( const Value& parameters );
    virtual void returnTo( const Value& parameters );

    virtual bool calling( String& strategyName, Value& parameters );
    virtual bool returning( String& nextStrategy, Value& parameters );

    virtual void run();

private:
    enum State
    {
        none,
        createForm,
        createFormError,
        createPlace,
        updateSelect,
        updateForm,
        updateFormError,
        updatePlace,
        editText,
        editTextDiscard,
        editTextSaveError,
        editTextError
    };

    void drawBackground();
    void drawSceneItemForm();
    bool drawEdit();
    void drawDiscard();
    void drawError( const String& message );
    void hideDialogue();

    void drawHotkeysCreatePlace();
    void drawHotkeysUpdateSelect();
    void drawHotkeysUpdatePlace();

    void start();
    void finish();

    bool closeEditor( bool save );

    void closeForm();

    InputDevice& m_inputDevice;
    WindowManager& m_windowManager;
    String m_mapWindowName;
    String m_formWindowName;
    Dialogue& m_dialogue;
    Hotkeys& m_hotkeys;
    AssetLoaders::Factory& m_assetLoaderFactory;
    Agape::Editor::Factory& m_editorFactory;
    const Metadata& m_worldMetadata;
    const Coordinates& m_coordinates;
    Compositor& m_compositor;

    enum State m_state;
    bool m_completed;

    Timer* m_timer;

    Terminal* m_mapTerminal;
    Terminal* m_formTerminal;
    Forms::Form* m_currentForm;
    Agape::Editor::Editor* m_editor;

    String m_assetName;
    String m_action;
    
    int m_row;
    int m_col;

    const SceneItem* m_updateSceneItem;
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_STRATEGIES_EDIT_WORLD_H
