#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "Assets/Asset.h"
#include "InputDevices/InputDevice.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "UI/Forms/Form.h"
#include "UI/Forms/FormTypes.h"
#include "UI/Dialogue.h"
#include "UI/Hotkeys.h"
#include "UI/Strategy.h"
#include "Utils/LiteStream.h"
#include "World/Compositor.h"
#include "World/SceneItem.h"
#include "Collections.h"
#include "EditorFactory.h"
#include "CarloStrategy.h"
#include "ProgramManager.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "WindowManager.h"

#include <math.h>

using Agape::String;

namespace
{
    // Settings for open template form.
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

    const char* backgroundAssetName( "large-dialogue" );

    const char* keyGuide( "\x1b[0m\x1b[97;100mC-S/A/Q\x1b[37m Save/As/Quick\x1b[0m  \x1b[97;100mEsc\x1b[37m Discard\x1b[0m" );
} // Anonymous namespace

using namespace Agape::InputDevices;

namespace Agape
{

namespace UI
{

namespace Strategies
{

Carlo::Carlo( InputDevice& inputDevice,
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
              Timers::Factory& timerFactory ) :
  m_inputDevice( inputDevice ),
  m_compositor( compositor ),
  m_editorFactory( editorFactory ),
  m_coordinates( coordinates ),
  m_programManager( programManager ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_dialogue( dialogue ),
  m_hotkeys( hotkeys ),
  m_windowManager( windowManager ),
  m_errorsWindowName( errorsWindowName ),
  m_formWindowName( formWindowName ),
  m_timer( timerFactory.makeTimer() ),
  m_completed( false ),
  m_state( none ),
  m_sceneItem( nullptr ),
  m_editingLinked( false ),
  m_editor( nullptr ),
  m_formTerminal( nullptr ),
  m_currentForm( nullptr ),
  m_needInvalidate( false )
{
}

Carlo::~Carlo()
{
    closeForm();
    delete( m_timer );
}

void Carlo::enter( const Value& parameters )
{
    m_sceneItem = m_compositor.selectLast();

    m_state = selectObject;
    m_completed = false;
    m_needInvalidate = false;

    m_compositor.lockScene( true );

    drawHotkeysSelectObject();

    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_formWindowName, terminalWindow ) )
    {
        m_formTerminal = terminalWindow.m_terminal;
    }
}

void Carlo::returnTo( const Value& parameters )
{
}

bool Carlo::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool Carlo::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        closeForm();
        m_hotkeys.clear();
        m_compositor.lockScene( false );
        return true;
    }

    return false;
}

void Carlo::run()
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
            else if( c == '\n' && m_sceneItem )
            {
                if( m_programManager.isFromTemplate( m_sceneItem->snowflake(), m_templateName ) )
                {
                    if( openTemplateEditor( true ) )
                    {
                        m_state = editProgram;
                        m_editingLinked = false;
                    }
                    else
                    {
                        m_completed = true;
                    }
                }
                else
                {
                    if( openEditor() )
                    {
                        m_state = editProgram;
                        m_editingLinked = true;
                    }
                    else
                    {
                        m_completed = true;
                    }
                }
                m_hotkeys.clear();
            }
            else if( c == 't' )
            {
                m_compositor.selectNone();
                m_compositor.setCursorsVisible( false );
                drawSelectProgram();
                m_hotkeys.clear();
                m_state = selectProgram;
            }
            else if( c == 's' )
            {
                if( openSceneProgEditor() )
                {
                    m_state = editProgram;
                    m_editingLinked = false;
                }
                else
                {
                    m_completed = true;
                }
                m_hotkeys.clear();
            }
            else if( c == 'd' )
            {
                if( openWorldProgEditor() )
                {
                    m_state = editProgram;
                    m_editingLinked = false;
                }
                else
                {
                    m_completed = true;
                }
                m_hotkeys.clear();
            }
            else if( c == Key::escape )
            {
                m_compositor.selectNone();
                m_completed = true;
                m_hotkeys.clear();
            }
            break;
        case selectProgram:
            if( c == '\n' )
            {
                if( m_currentForm )
                {
                    m_templateName = m_currentForm->getFieldContents( _Name );
                    closeForm();
                    hideFormWindow();
                    if( openTemplateEditor( false ) )
                    {
                        m_state = editProgram;
                        m_editingLinked = false;
                    }
                    else
                    {
                        drawSelectProgramError();
                        m_state = selectProgramError;
                    }
                }
            }
            else if( c == Key::escape )
            {
                hideFormWindow();
                m_compositor.setCursorsVisible( true );
                m_completed = true;
            }
            else
            {
                if( m_currentForm )
                {
                    m_currentForm->consumeChar( c );
                }
            }
            break;
        case selectProgramError:
            if( ( c == '\n' ) || ( c == Key::escape ) )
            {
                hideDialogue();
                drawSelectProgram();
                m_state = selectProgram;
            }
            break;
        case editProgram:
            if( c == control( 's' ) )
            {
                if( closeEditor( true ) )
                {
                    afterClose();
                    m_completed = true;
                }
                else
                {
                    drawSaveError();
                    m_state = saveError;
                }
            }
            else if( c == control( 'q' ) && m_editor )
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
            else if( ( c == control( 'a' ) ) && m_editor )
            {
                drawSaveAs();
                m_state = saveAs;
            }
            else if( c == Key::escape )
            {
                if( m_editor && m_editor->modified() )
                {
                    drawDiscard();
                    m_state = discard;
                }
                else
                {
                    closeEditor( false );
                    afterClose();
                    m_completed = true;
                }
            }
            else if( m_editor )
            {
                m_editor->consumeCharacter( c );
            }
            break;
        case saveAs:
            if( c == '\n' &&
                m_currentForm &&
                m_editor )
            {
                // Set program name as new from form. This will then be the
                // program we insert after a successful save and will be the
                // asset we send an invalidation notification for.
                m_programName = m_currentForm->getFieldContents( _Name );
                m_editingLinked = false;
                closeForm();
                hideFormWindow();
                if( closeEditor( true, m_programName ) )
                {
                    afterClose();
                    m_completed = true;
                }
                else
                {
                    drawSaveError();
                    m_state = saveError;
                }
            }
            else if( c == '\x1b' )
            {
                closeForm();
                hideFormWindow();
                if( m_editor ) m_editor->draw();
                m_state = editProgram;
            }
            else if( m_currentForm )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        case discard:
            if( c == 'y' )
            {
                hideDialogue();
                closeEditor( false );
                afterClose();
                m_completed = true;
            }
            else
            {
                hideDialogue();
                m_state = editProgram;
            }
            break;
        case saveError:
            if( c == '\n' )
            {
                hideDialogue();
                m_state = editProgram;
            }
            break;
        default:
            break;
        }
    }

    // FIXME: Run compositor and MIDIPlayer from Session as required, rather
    // than from each strategy?
    m_compositor.run();

    if( ( m_state == editProgram ) && m_editor )
    {
        m_editor->run();
    }
    else if( m_state == quickSaveSuccess )
    {
        if( m_timer->ms() >= 1000 )
        {
            hideDialogue();
            m_state = editProgram;
        }
    }
}

bool Carlo::openEditor()
{
    m_compositor.selectNone();
    m_compositor.setCursorsVisible( false );

    m_windowManager.setTerminalWindowVisible( m_errorsWindowName, true );

    LiteStream stream;
    stream << "Carlo-Linda: " << m_sceneItem->assetName() << " at " << m_sceneItem->row() << "," << m_sceneItem->col();

    m_programName = m_sceneItem->snowflake();
    m_instanceName = m_programName;
    m_displayName = stream.str();
    m_linkedItemName = m_sceneItem->snowflake();

    m_editor = m_editorFactory.makeEditor( m_coordinates,
                                           m_programName,
                                           m_instanceName,
                                           m_displayName,
                                           m_linkedItemName,
                                           keyGuide );
    
    if( m_editor->open( Editor::Editor::modeWrite | Editor::Editor::modeCreate ) )
    {
        m_editor->draw( true ); // true = highlight
        return true;
    }

    return false;
}

bool Carlo::openTemplateEditor( bool withItem )
{
    bool success( false );

    m_compositor.selectNone();
    m_compositor.setCursorsVisible( false );

    m_windowManager.setTerminalWindowVisible( m_errorsWindowName, true );

    m_programName = m_templateName;

    LiteStream stream;
    if( withItem )
    {
        m_instanceName = m_sceneItem->snowflake();
        stream << "Carlo-Linda: " << m_sceneItem->assetName() << " at " << m_sceneItem->row() << "," << m_sceneItem->col() << " (template " << m_templateName << ")";
    }
    else
    {
        m_instanceName.clear();
        stream << "Carlo-Linda: " << m_templateName;
    }

    m_displayName = stream.str();
    m_linkedItemName.clear();

    m_editor = m_editorFactory.makeEditor( m_coordinates,
                                           m_programName,
                                           m_instanceName,
                                           m_displayName,
                                           m_linkedItemName,
                                           keyGuide );

    if( m_editor->open( Editor::Editor::modeWrite | Editor::Editor::modeCreate ) )
    {
        m_editor->draw( true ); // true = highlight
        success = true;
    }

    return success;
}

bool Carlo::openSceneProgEditor()
{
    m_compositor.selectNone();
    m_compositor.setCursorsVisible( false );

    m_windowManager.setTerminalWindowVisible( m_errorsWindowName, true );

    LiteStream stream;
    stream << "Carlo-Linda: Scene program for "
           << ::abs( m_coordinates.m_y );
    if( m_coordinates.m_y >= 0 )
    {
        stream << "N ";
    }
    else
    {
        stream << "S ";
    }
    stream << ::abs( m_coordinates.m_x );
    if( m_coordinates.m_x >= 0 )
    {
        stream << "E";
    }
    else
    {
        stream << "W";
    }
    LiteStream filenameStream;
    
    filenameStream << _scene_ << m_coordinates.m_x << "_" << m_coordinates.m_y;

    m_programName = filenameStream.str();
    m_instanceName = m_programName;
    m_displayName = stream.str();
    m_linkedItemName.clear();

    m_editor = m_editorFactory.makeEditor( m_coordinates,
                                           m_programName,
                                           m_instanceName,
                                           m_displayName,
                                           m_linkedItemName,
                                           keyGuide );

    if( m_editor->open( Editor::Editor::modeWrite | Editor::Editor::modeCreate ) )
    {
        m_editor->draw( true ); // true = highlight
        return true;
    }

    return false;
}

bool Carlo::openWorldProgEditor()
{
    m_compositor.selectNone();
    m_compositor.setCursorsVisible( false );

    m_windowManager.setTerminalWindowVisible( m_errorsWindowName, true );

    m_programName = _World;
    m_instanceName = m_programName;
    m_displayName = "Carlo-Linda: World program";
    m_linkedItemName.clear();

    m_editor = m_editorFactory.makeEditor( m_coordinates,
                                           m_programName,
                                           m_instanceName,
                                           m_displayName,
                                           m_linkedItemName,
                                           keyGuide );

    if( m_editor->open( Editor::Editor::modeWrite | Editor::Editor::modeCreate ) )
    {
        m_editor->draw( true ); // true = highlight
        return true;
    }

    return false;
}

bool Carlo::quickSave()
{
    int offset = 0;
    int row = 0;
    int col = 0;
    m_editor->getPosition( offset, row, col );

    if( m_editor->close( true ) )
    {
        delete( m_editor );

        m_editor = m_editorFactory.makeEditor( m_coordinates,
                                                m_programName,
                                                m_instanceName,
                                                m_displayName,
                                                m_linkedItemName,
                                                keyGuide );

        if( m_editor->open( Editor::Editor::modeWrite | Editor::Editor::modeCreate ) )
        {
            m_editor->setPosition( offset, row, col );
            m_editor->draw( true, false ); // true = highlight, false = don't repaginate (this is done for setPosition()).
        }

        m_needInvalidate = true;

        return true;
    }

    return false;
}

bool Carlo::closeEditor( bool save, const String& as )
{
    if( m_editor && m_editor->close( save, as ) )
    {
        if( save ) m_needInvalidate = true;

        delete( m_editor );
        m_editor = nullptr;

        return true;
    }

    return false;
}

bool Carlo::closeEditor( bool save )
{
    return closeEditor( save, String() );
}

void Carlo::afterClose()
{
    m_windowManager.setTerminalWindowVisible( m_errorsWindowName, false );

    m_compositor.setCursorsVisible( true );

    if( m_editingLinked )
    {
        m_compositor.setItemFlags( m_sceneItem->snowflake(),
                                   (SceneItem::Flags)( m_sceneItem->flags() | SceneItem::linkedProgram ),
                                   true ); // true = update local scene immediately.
    }

    m_compositor.lockScene( false );
    bool didRender( false );
    if( m_needInvalidate )
    {
        // Notify everyone else in the world that we changed the
        // asset so they can redraw it if necessary. Also redraws
        // the current scene for us, if needed.
        m_compositor.notifyInvalidated( m_programName,
                                        _program,
                                        didRender );
    }
    // If the compositor didn't force a redraw (or we didn't ask because
    // we didn't modify the asset), we still need to do it, as the editor
    // shares the same terminal as the world.
    if( !didRender ) m_compositor.render();
}

void Carlo::drawSelectProgram()
{
    AssetLoaders::Baked assetLoader( World::Coordinates(), backgroundAssetName );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        m_formTerminal->consumeNext( 0, 0 );
        m_formTerminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
    }

    m_formTerminal->consumeNext( contentFirstRow, contentCol );
    m_formTerminal->consumeString( "Enter the name of a template to open or create.", false, Terminal::preserveBackground );

    Vector< Forms::Field > fields;
    Forms::Field nameField( _Name,
                            textWidth,
                            textHeight,
                            contentFirstRow + 2,
                            contentCol + 7,
                            fieldAttributes,
                            labelAttributes,
                            contentFirstRow + 2,
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
                                     Forms::Title( _Open_Template,
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

void Carlo::drawSelectProgramError()
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( "The requested program could not be found" );
}

void Carlo::drawSaveAs()
{
    AssetLoaders::Baked assetLoader( World::Coordinates(), backgroundAssetName );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        m_formTerminal->consumeNext( 0, 0 );
        m_formTerminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
    }

    m_formTerminal->printFormatted( "Enter the name to save this program as.", contentFirstRow, contentCol, 2, contentMaxWidth, Terminal::noHCentre, Terminal::noVCentre, contentAttributes, Terminal::preserveBackground );

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
    nameField.m_contents = m_programName;
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

void Carlo::drawQuickSaveSuccess()
{
    const char* message( "Saved" );

    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( message );
}

void Carlo::drawDiscard()
{
    const char* message( "Are you sure you want to discard your changes?\
                          Hit \x1b[97mY\x1b[0m to confirm, or\
                          any other key to go back." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Are you sure?" );
    m_dialogue.drawMessage( message );
}

void Carlo::drawSaveError()
{
    const char* message( "Your changes could not be saved.\
                          Hit \x1b[97mRet\x1b[0m to go back, then\
                          \x1b[97mC-S\x1b[0m to try saving again." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void Carlo::hideDialogue()
{
    m_dialogue.hide();
}

void Carlo::hideFormWindow()
{
    m_windowManager.setTerminalWindowVisible( m_formWindowName, false );
}

void Carlo::drawHotkeysSelectObject()
{
    m_hotkeys.clear();
    m_hotkeys.show( "\x18\x19\x1B\x1A", "Select" );
    m_hotkeys.show( "Tab", "Next" );
    m_hotkeys.show( "Sh-Tab", "Prev" );
    m_hotkeys.spacer();
    m_hotkeys.show( "Ret", "Edit" );
    m_hotkeys.show( "T", "emplate" );
    m_hotkeys.show( "S", "ceneProg" );
    m_hotkeys.show( "D", "WorldProg" );
    m_hotkeys.spacer();
    m_hotkeys.show( "Esc", "Cancel" );
}

void Carlo::closeForm()
{
    if( m_currentForm )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
