#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "InputDevices/InputDevice.h"
#include "Timers/Factories/TimerFactory.h"
#include "UI/Dialogue.h"
#include "UI/Hotkeys.h"
#include "World/Compositor.h"
#include "World/WorldCoordinates.h"
#include "ANSIEditorFactory.h"
#include "ANSIEditorStrategy.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"

using Agape::String;

namespace
{
    // Settings for select program form.
    const int titleRow( 0 );
    const int titleAttributes( 0x9F );

    const int contentFirstRow( 2 );
    const int contentCol( 1 );
    const int contentMaxWidth( 51 );
    const int contentAttributes( 0x07 );

    const int labelAttributes( 0x17 );

    const int fieldAttributes( 0x07 );

    const int textWidth( 29 );
    const int textHeight( 1 );

    const int guideRow( 15 );

    const String backgroundAssetName( "large-dialogue" );
} // Anonymous namespace

using namespace Agape::InputDevices;
using namespace Agape::World;

namespace Agape
{

namespace UI
{

namespace Strategies
{

ANSIEditor::ANSIEditor( InputDevice& inputDevice,
                        Compositor& compositor,
                        Agape::ANSIEditor::Factory& ansiEditorFactory,
                        AssetLoaders::Factory& assetLoaderFactory,
                        AssetLoaders::Factory& programAssetLoaderFactory,
                        const World::Coordinates& coordinates,
                        Dialogue& dialogue,
                        Hotkeys& hotkeys,
                        WindowManager& windowManager,
                        const String& formWindowName,
                        Timers::Factory& timerFactory ) :
  m_inputDevice( inputDevice ),
  m_compositor( compositor ),
  m_ansiEditorFactory( ansiEditorFactory ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_programAssetLoaderFactory( programAssetLoaderFactory ),
  m_coordinates( coordinates ),
  m_dialogue( dialogue ),
  m_hotkeys( hotkeys ),
  m_windowManager( windowManager ),
  m_formWindowName( formWindowName ),
  m_timer( timerFactory.makeTimer() ),
  m_completed( false ),
  m_state( none ),
  m_sceneItem( nullptr ),
  m_currentForm( nullptr ),
  m_ansiEditor( nullptr ),
  m_prevFGColour( 0x07 ),
  m_prevBGColour( 0x00 ),
  m_doInsert( false ),
  m_editingHeights( false ),
  m_needInvalidate( false )
{
}

ANSIEditor::~ANSIEditor()
{
    closeForm();
    delete( m_timer );
    delete( m_ansiEditor );
}

void ANSIEditor::enter( const Value& parameters )
{
    m_sceneItem = m_compositor.selectLast();

    m_completed = false;
    m_doInsert = false;
    m_editingHeights = false;
    m_needInvalidate = false;

    m_compositor.lockScene( true );

    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_formWindowName, terminalWindow ) )
    {
        m_formTerminal = terminalWindow.m_terminal;
    }

    if( m_sceneItem )
    {
        drawHotkeysSelectObject();
        m_state = selectObject;
    }
    else
    {
        m_compositor.selectNone();
        m_compositor.setCursorsVisible( false );
        drawSelectAsset();
        m_state = selectAsset;
    }
}

void ANSIEditor::returnTo( const Value& parameters )
{
}

bool ANSIEditor::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool ANSIEditor::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        closeForm();
        
        delete( m_ansiEditor );
        m_ansiEditor = nullptr;
        
        m_hotkeys.clear();
        
        m_compositor.lockScene( false );
        
        if( m_doInsert )
        {
            nextStrategy = _edit;
            parameters[_mode] = _insert;
            parameters[_assetName] = m_assetName;
        }
        
        return true;
    }

    return false;
}

void ANSIEditor::run()
{
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == Key::carriageReturn ) // Eat all CRs.
        {
            continue;
        }

        switch( m_state )
        {
        case selectObject:
            if( c == Key::tab )
            {
                m_sceneItem = m_compositor.selectNext();
            }
            else if( c == Key::shiftTab )
            {
                m_sceneItem = m_compositor.selectPrevious();
            }
            else if( c == Key::up )
            {
                const SceneItem* nextSceneItem( m_compositor.selectDirection( Direction::up ) );
                if( nextSceneItem != NULL )
                {
                    m_sceneItem = nextSceneItem;
                }
            }
            else if( c == Key::down )
            {
                const SceneItem* nextSceneItem( m_compositor.selectDirection( Direction::down ) );
                if( nextSceneItem != NULL )
                {
                    m_sceneItem = nextSceneItem;
                }
            }
            else if( c == Key::left )
            {
                const SceneItem* nextSceneItem( m_compositor.selectDirection( Direction::left ) );
                if( nextSceneItem != NULL )
                {
                    m_sceneItem = nextSceneItem;
                }
            }
            else if( c == Key::right )
            {
                const SceneItem* nextSceneItem( m_compositor.selectDirection( Direction::right ) );
                if( nextSceneItem != NULL )
                {
                    m_sceneItem = nextSceneItem;
                }
            }
            else if( c == '\n' )
            {
                if( m_sceneItem )
                {
                    m_assetName = m_sceneItem->assetName();
                    m_compositor.selectNone();
                    m_compositor.setCursorsVisible( false );
                    openEditor();
                    m_state = editing;
                }
            }
            else if( c == 'o' )
            {
                m_compositor.selectNone();
                m_compositor.setCursorsVisible( false );
                drawSelectAsset();
                m_hotkeys.clear();
                m_state = selectAsset;
            }
            else if( c == Key::escape )
            {
                m_compositor.selectNone();
                m_completed = true;
                m_hotkeys.clear();
            }
            break;
        case selectAsset:
            if( c == '\n' &&
                m_currentForm )
            {
                setAssetName();
                hideFormWindow();
                openEditor();
                m_state = editing;
            }
            else if( c == '\x1b' )
            {
                closeForm();
                hideFormWindow();
                m_compositor.selectNone();
                m_compositor.setCursorsVisible( true );
                m_completed = true;
                m_hotkeys.clear();
            }
            else if( m_currentForm )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        case editing:
            if( ( c == control( 'm' ) ) && m_ansiEditor )
            {
                drawSelectTemplate();
                m_state = selectTemplate;
            }
            else if( ( c == control( 's' ) ) && m_ansiEditor )
            {
                if( closeEditor( true ) )
                {
                    afterClose();
                    m_state = none;
                    m_completed = true;
                }
                else
                {
                    drawSaveError();
                    m_state = saveError;
                }
            }
            else if( ( c == control( 'q' ) ) && m_ansiEditor )
            {
                if( quickSave() )
                {
                    drawQuickSaveSuccess();
                    m_state = quickSaveSuccess;
                    m_timer->reset();
                }
                else
                {
                    drawSaveError();
                    m_state = saveError;
                }
            }
            else if( ( c == control( 'a' ) ) && m_ansiEditor )
            {
                drawSaveAs();
                m_state = saveAs;
            }
            else if( ( c == '\x1b' ) && m_ansiEditor )
            {
                if( m_ansiEditor->modified() )
                {
                    drawDiscard();
                    m_state = discard;
                }
                else
                {
                    closeEditor( false );
                    afterClose();
                    m_doInsert = false;
                    m_state = none;
                    m_completed = true;
                }
            }
            else if( m_ansiEditor )
            {
                if( c == control( 'h' ) )
                {
                    // Snoop this key, then pass through to editor.
                    if( !m_editingHeights )
                    {
                        m_editingHeights = true;
                        drawHotkeysEditingHeights();
                    }
                    else
                    {
                        m_editingHeights = false;
                        drawHotkeysEditing();
                    }
                }
                m_ansiEditor->consumeCharacter( c );
            }
            break;
        case selectTemplate:
            if( c == '\n' &&
                m_currentForm &&
                m_ansiEditor )
            {
                if( setTemplate() )
                {
                    drawSelectTemplateSuccess();
                    m_state = selectTemplateSuccess;
                    m_timer->reset();
                }
                else
                {
                    drawSelectTemplateError();
                    m_state = selectTemplateError;
                    m_timer->reset();
                }
            }
            else if( c == '\x1b' )
            {
                closeForm();
                hideFormWindow();
                if( m_ansiEditor ) m_ansiEditor->redraw();
                m_state = editing;
            }
            else if( m_currentForm )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        case saveAs:
            if( c == '\n' &&
                m_currentForm &&
                m_ansiEditor )
            {
                // Set asset name as new from form. This will then be the asset
                // we insert after a successful save and will be the asset we
                // send an invalidation notification for.
                setAssetName();
                if( closeEditor( true, m_assetName ) )
                {
                    closeForm();
                    hideFormWindow();
                    afterClose();
                    m_doInsert = true; // Always insert new after save as
                    m_state = none;
                    m_completed = true;
                }
                else
                {
                    closeForm();
                    hideFormWindow();
                    drawSaveError();
                    m_state = saveError;
                }
            }
            else if( c == '\x1b' )
            {
                closeForm();
                hideFormWindow();
                if( m_ansiEditor ) m_ansiEditor->redraw();
                m_state = editing;
            }
            else if( m_currentForm )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        case discard:
            if( ( c == 'y' ) && m_ansiEditor )
            {
                hideDialogue();
                closeEditor( false );
                m_compositor.setCursorsVisible( true );
                m_compositor.render();
                m_doInsert = false;
                m_state = none;
                m_completed = true;
            }
            else
            {
                hideDialogue();
                if( m_ansiEditor ) m_ansiEditor->redraw();
                m_state = editing;
            }
            break;
        case saveError:
            if( c == '\n' )
            {
                hideDialogue();
                if( m_ansiEditor ) m_ansiEditor->redraw();
                m_state = editing;
            }
            break;
        default:
            break;
        }
    }

    m_compositor.run();

    if( ( m_state == editing ) && m_ansiEditor )
    { 
        m_ansiEditor->run();
    }
    else if( m_state == selectTemplateSuccess )
    {
        if( m_timer->ms() >= 2000 )
        {
            hideDialogue();
            closeForm();
            hideFormWindow();
            if( m_ansiEditor ) m_ansiEditor->redraw();
            m_state = editing;
        }
    }
    else if( m_state == selectTemplateError )
    {
        if( m_timer->ms() >= 2000 )
        {
            hideDialogue();
            m_state = selectTemplate;
        }
    }
    else if( m_state == quickSaveSuccess )
    {
        if( m_timer->ms() >= 1000 )
        {
            hideDialogue();
            m_state = editing;
        }
    }
}

bool ANSIEditor::openEditor()
{
    // See if this is a new (not currently existing) asset - if so, flag that
    // we need to offer to the user to insert it into the scene when we return.
    AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, m_assetName ) );
    m_doInsert = !assetLoader->open();
    assetLoader->close();
    delete( assetLoader );

    m_ansiEditor = m_ansiEditorFactory.makeEditor( m_coordinates, m_assetName );

    m_ansiEditor->open( Agape::ANSIEditor::ANSIEditor::modeWrite |
                        Agape::ANSIEditor::ANSIEditor::modeCreate );
    m_ansiEditor->setFGColour( m_prevFGColour );
    m_ansiEditor->setBGColour( m_prevBGColour );
    m_ansiEditor->draw();

    drawHotkeysEditing();

    m_state = editing;

    return true;
}

bool ANSIEditor::quickSave()
{
    if( m_ansiEditor->save() )
    {
        m_needInvalidate = true;
        return true;
    }

    return false;
}

bool ANSIEditor::closeEditor( bool save, const String& as )
{
    m_prevFGColour = m_ansiEditor->fgColour();
    m_prevBGColour = m_ansiEditor->bgColour();
    if( m_ansiEditor->close( save, as ) )
    {
        if( save ) m_needInvalidate = true;
        return true;
    }

    return false;
}

bool ANSIEditor::closeEditor( bool save )
{
    m_prevFGColour = m_ansiEditor->fgColour();
    m_prevBGColour = m_ansiEditor->bgColour();
    if( m_ansiEditor->close( save ) )
    {
        if( save ) m_needInvalidate = true;
        return true;
    }

    return false;
}

void ANSIEditor::afterClose()
{
    m_compositor.setCursorsVisible( true );

    m_compositor.lockScene( false );
    bool didRender( false );
    if( m_needInvalidate )
    {
        // Notify everyone else in the world that we changed the
        // asset so they can redraw it if necessary. Also redraws
        // the current scene for us, if needed.
        m_compositor.notifyInvalidated( m_assetName,
                                        _asset,
                                        didRender );
    }
    // If the compositor didn't force a redraw (or we didn't ask because
    // we didn't modify the asset), we still need to do it, as the ANSIEditor
    // shares the same terminal as the world.
    if( !didRender ) m_compositor.render();
}

void ANSIEditor::drawSelectAsset()
{
    AssetLoaders::Baked assetLoader( World::Coordinates(), backgroundAssetName );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        m_formTerminal->consumeNext( 0, 0 );
        m_formTerminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
    }

    m_formTerminal->printFormatted( "Enter the name of the artwork you wish to edit, or a name for the artwork you wish to create.", contentFirstRow, contentCol, 2, contentMaxWidth, Terminal::noHCentre, Terminal::noVCentre, contentAttributes, Terminal::preserveBackground );

    Vector< Forms::Field > fields;
    Forms::Field nameField( _Name,
                            textWidth,
                            textHeight,
                            contentFirstRow + 3,
                            contentCol + 7,
                            fieldAttributes,
                            labelAttributes,
                            contentFirstRow + 3,
                            contentCol );
    nameField.m_permittedSymbols = "-_";
    fields.push_back( nameField );

    m_formTerminal->consumeNext( guideRow, contentCol );
    m_formTerminal->consumeString( "\x1b[97;100mRet\x1b[37m Open\x1b[0m  \x1b[97;100mEsc\x1b[37m Cancel\x1b[0m", false, Terminal::preserveBackground );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                     m_formWindowName,
                                     String(),
                                     fields,
                                     Forms::Title( _Open_Or_Create,
                                                   titleRow,
                                                   0,
                                                   contentMaxWidth,
                                                   true,
                                                   titleAttributes ) );

    m_currentForm->openUI();
    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->reset();
        m_currentForm->draw();
    }

    m_windowManager.setTerminalWindowVisible( m_formWindowName, true );
}

void ANSIEditor::drawSelectTemplate()
{
    AssetLoaders::Baked assetLoader( World::Coordinates(), backgroundAssetName );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        m_formTerminal->consumeNext( 0, 0 );
        m_formTerminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
    }

    m_formTerminal->printFormatted( "Enter the name of a program template to associate with this artwork.", contentFirstRow, contentCol, 2, contentMaxWidth, Terminal::noHCentre, Terminal::noVCentre, contentAttributes, Terminal::preserveBackground );

    Vector< Forms::Field > fields;
    Forms::Field nameField( _Name,
                            textWidth,
                            textHeight,
                            contentFirstRow + 3,
                            contentCol + 7,
                            fieldAttributes,
                            labelAttributes,
                            contentFirstRow + 3,
                            contentCol );
    nameField.m_permittedSymbols = "-_";
    nameField.m_contents = m_ansiEditor->getTemplateName();
    fields.push_back( nameField );

    m_formTerminal->consumeNext( guideRow, contentCol );
    m_formTerminal->consumeString( "\x1b[97;100mRet\x1b[37m Associate\x1b[0m  \x1b[97;100mEsc\x1b[37m Cancel\x1b[0m", false, Terminal::preserveBackground );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                     m_formWindowName,
                                     String(),
                                     fields,
                                     Forms::Title( _Set_Template,
                                                   titleRow,
                                                   0,
                                                   contentMaxWidth,
                                                   true,
                                                   titleAttributes ) );

    m_currentForm->openUI();
    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->draw();
    }

    m_windowManager.setTerminalWindowVisible( m_formWindowName, true );
}

void ANSIEditor::drawSelectTemplateSuccess()
{
    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( "Program template set" );
}

void ANSIEditor::drawSelectTemplateError()
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( "The requested program template could not be found" );
}

void ANSIEditor::drawSaveAs()
{
    AssetLoaders::Baked assetLoader( World::Coordinates(), backgroundAssetName );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        m_formTerminal->consumeNext( 0, 0 );
        m_formTerminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
    }

    m_formTerminal->printFormatted( "Enter the name to save this artwork as.", contentFirstRow, contentCol, 2, contentMaxWidth, Terminal::noHCentre, Terminal::noVCentre, contentAttributes, Terminal::preserveBackground );

    Vector< Forms::Field > fields;
    Forms::Field nameField( _Name,
                            textWidth,
                            textHeight,
                            contentFirstRow + 3,
                            contentCol + 7,
                            fieldAttributes,
                            labelAttributes,
                            contentFirstRow + 3,
                            contentCol );
    nameField.m_permittedSymbols = "-_";
    nameField.m_contents = m_ansiEditor->assetName();
    fields.push_back( nameField );

    m_formTerminal->consumeNext( guideRow, contentCol );
    m_formTerminal->consumeString( "\x1b[97;100mRet\x1b[37m Save as\x1b[0m  \x1b[97;100mEsc\x1b[37m Cancel\x1b[0m", false, Terminal::preserveBackground );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                     m_formWindowName,
                                     String(),
                                     fields,
                                     Forms::Title( _Save_As,
                                                   titleRow,
                                                   0,
                                                   contentMaxWidth,
                                                   true,
                                                   titleAttributes ) );

    m_currentForm->openUI();
    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->draw();
    }

    m_windowManager.setTerminalWindowVisible( m_formWindowName, true );
}

void ANSIEditor::drawQuickSaveSuccess()
{
    const char* message( "Saved" );

    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( message );
}

void ANSIEditor::drawDiscard()
{
    const char* message( "Are you sure you want to discard your changes?\
                          Hit \x1b[97mY\x1b[0m to confirm, or\
                          any other key to go back." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Are you sure?" );
    m_dialogue.drawMessage( message );
}

void ANSIEditor::drawSaveError()
{
    const char* message( "Your changes could not be saved.\
                          Hit \x1b[97mRet\x1b[0m to go back, then\
                          \x1b[97mC-S\x1b[0m to try saving again." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void ANSIEditor::hideDialogue()
{
    m_dialogue.hide();
}

void ANSIEditor::hideFormWindow()
{
    m_windowManager.setTerminalWindowVisible( m_formWindowName, false );
}

void ANSIEditor::drawHotkeysSelectObject()
{
    m_hotkeys.clear();
    m_hotkeys.show( "\x18\x19\x1B\x1A", "Select" );
    m_hotkeys.show( "Tab", "Next" );
    m_hotkeys.show( "Sh-Tab", "Prev" );
    m_hotkeys.show( "Ret", "Edit" );
    m_hotkeys.show( "O", "pen/Creat" );
    m_hotkeys.show( "Esc", "Cancel" );
}

void ANSIEditor::drawHotkeysEditing()
{
    m_hotkeys.clear();
    m_hotkeys.show( "\x18\x19\x1B\x1A", "Move" );
    m_hotkeys.show( "C-\x18\x19\x1B\x1A", "Clr" );
    m_hotkeys.show( "Tab", "Set1" );
    m_hotkeys.show( "C-0..9", "Set1" );
    m_hotkeys.show( "C-O", "LitTgl" );
    m_hotkeys.show( "Sh-Tab", "Set2" );
    m_hotkeys.spacer();
    m_hotkeys.show( "C-'/", "Y+/-" );
    m_hotkeys.show( "C-.,", "X+/-" );
    m_hotkeys.show( "C-H", "eight" );
    m_hotkeys.spacer();
    m_hotkeys.show( "Sh-\x18\x19\x1B\x1A", "Sel" );
    m_hotkeys.show( "C-X", "Cut" );
    m_hotkeys.show( "C-C", "opy" );
    m_hotkeys.show( "C-V", "Paste" );
    m_hotkeys.show( "C-Z", "Undo" );
    m_hotkeys.spacer();
    m_hotkeys.show( "C-LK", "Ani+/-" );
    m_hotkeys.show( "C-B", "Blit" );
    m_hotkeys.show( "C-R", "Sprite" );
    m_hotkeys.show( "C-MJ", "Templ" );
    m_hotkeys.spacer();
    m_hotkeys.show( "C-SA", "ve(as)" );
    m_hotkeys.show( "C-Q", "uickSv" );
    m_hotkeys.show( "Esc", "Cancel" );
}

void ANSIEditor::drawHotkeysEditingHeights()
{
    m_hotkeys.clear();
    m_hotkeys.show( "\x18\x19\x1B\x1A", "Move" );
    m_hotkeys.spacer();
    m_hotkeys.show( "F", "oregnd" );
    m_hotkeys.show( "B", "kgnd" );
    m_hotkeys.show( "G", "nd" );
    m_hotkeys.spacer();
    m_hotkeys.show( "C-H", "Back" );
    m_hotkeys.spacer();
    m_hotkeys.show( "C-S", "ave" );
    m_hotkeys.show( "Esc", "Cancel" );
}

void ANSIEditor::closeForm()
{
    if( m_currentForm )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

void ANSIEditor::setAssetName()
{
    if( m_currentForm )
    {
        m_assetName = m_currentForm->getFieldContents( _Name );
    }
}

bool ANSIEditor::setTemplate()
{
    if( m_currentForm )
    {
        String programTemplateName( m_currentForm->getFieldContents( _Name ) );
        AssetLoader* programAssetLoader( m_programAssetLoaderFactory.makeLoader( m_coordinates,
                                                                                 programTemplateName ) );
        if( programAssetLoader->open() )
        {
            m_ansiEditor->setTemplateName( programTemplateName );
            return true;
        }

        delete( programAssetLoader );
    }

    return false;
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
