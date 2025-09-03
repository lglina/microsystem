#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "Assets/Asset.h"
#include "Clocks/Clock.h"
#include "InputDevices/InputDevice.h"
#include "Timers/Factories/TimerFactory.h"
#include "UI/Forms/Form.h"
#include "UI/Dialogue.h"
#include "UI/Hotkeys.h"
#include "World/Compositor.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "Editor.h"
#include "EditorFactory.h"
#include "EditWorldStrategy.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"

using namespace Agape::InputDevices;
using namespace Agape::World;

using Agape::String;

namespace
{
    const int titleRow( 0 );
    const int titleAttributes( 0x9F );

    const int contentFirstRow( 2 );
    const int contentCol( 1 );
    const int contentMaxWidth( 51 );
    const int contentAttributes( 0x07 );

    const int labelAttributes( 0x07 );

    const int fieldAttributes( 0x07 );
    const int disabledFieldAttributes( 0x17 );
    
    const int textWidth( 38 );
    const int textHeight( 1 );
    const int textMultiHeight( 2 );

    const int guideRow( 15 );

    const String backgroundAssetName( "large-dialogue" );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

EditWorld::EditWorld( InputDevice& inputDevice,
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
                      Compositor& compositor ) :
  m_inputDevice( inputDevice ),
  m_windowManager( windowManager ),
  m_mapWindowName( mapWindowName ),
  m_formWindowName( formWindowName ),
  m_dialogue( dialogue ),
  m_hotkeys( hotkeys ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_editorFactory( editorFactory ),
  m_worldMetadata( worldMetadata ),
  m_coordinates( coordinates ),
  m_compositor( compositor ),
  m_state( none ),
  m_completed( false ),
  m_timer( timerFactory.makeTimer() ),
  m_mapTerminal( nullptr ),
  m_formTerminal( nullptr ),
  m_currentForm( nullptr ),
  m_editor( nullptr ),
  m_row( 0 ),
  m_col( 0 ),
  m_updateSceneItem( nullptr )
{
}

EditWorld::~EditWorld()
{
    closeForm();
    delete( m_timer );
    delete( m_editor );
}

void EditWorld::enter( const Value& parameters )
{
    m_completed = false;

    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_mapWindowName, terminalWindow ) )
    {
        m_mapTerminal = terminalWindow.m_terminal;
    }
    if( m_windowManager.getTerminalWindow( m_formWindowName, terminalWindow ) )
    {
        m_formTerminal = terminalWindow.m_terminal;
    }

    if( m_mapTerminal && m_formTerminal )
    {
        if( parameters[_mode] == String( _insert ) )
        {
            if( !parameters.hasValue( _assetName ) )
            {
                m_state = createForm;
                start();
                drawSceneItemForm();
            }
            else
            {
                start();
                // FIXME: Copypasta from below.
                m_row = m_compositor.positionRow();
                m_col = m_compositor.positionCol();
                m_compositor.createSprite( "Edit", parameters[_assetName], m_row, m_col );
                m_state = createPlace;
                drawHotkeysCreatePlace();
            }
        }
        else if( parameters[_mode] == String( _update ) )
        {
            m_updateSceneItem = m_compositor.selectLast();
            if( m_updateSceneItem )
            {
                m_state = updateSelect;
                drawHotkeysUpdateSelect();
                start();
            }
            else
            {
                m_state = none;
                m_completed = true;
            }
        }
        else
        {
            m_state = none;
            m_completed = true;
        }
    }
    else
    {
        m_state = none;
        m_completed = true;
    }
}

void EditWorld::returnTo( const Value& parameters )
{
}

bool EditWorld::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool EditWorld::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        closeForm();
        m_hotkeys.clear();
        return true;
    }

    return false;
}

void EditWorld::run()
{
    m_compositor.run();

    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }
    
        switch( m_state )
        {
        case createForm:
            if( c == '\n' )
            {
                if( m_currentForm )
                {
                    m_assetName = m_currentForm->getFieldContents( "Name" );
                    m_action = m_currentForm->getFieldContents( "Action" );
                    
                    AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, m_assetName ) );
                    if( assetLoader->open() )
                    {
                        delete( assetLoader );

                        closeForm();
                        m_windowManager.setTerminalWindowVisible( m_formWindowName, false );

                        m_row = m_compositor.positionRow();
                        m_col = m_compositor.positionCol();
                        // FIXME: Don't place sprite right over user, but could go off edge of screen now?
                        m_compositor.createSprite( "Edit", m_assetName, m_row, m_col );
                        m_state = createPlace;
                        drawHotkeysCreatePlace();
                    }
                    else
                    {
                        delete( assetLoader );
                        drawError( "No such item" );
                        m_timer->reset();
                        m_state = createFormError;
                    }
                }
            }
            else if( c == Key::escape )
            {
                closeForm();
                m_windowManager.setTerminalWindowVisible( m_formWindowName, false );
                finish();
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
        case createPlace:
            if( c == Key::up )
            {
                if( m_row > 0 )
                {
                    --m_row;
                    m_compositor.moveSprite( "Edit", m_row, m_col );
                }
            }
            else if( c == Key::down )
            {
                if( m_row < m_mapTerminal->height() )
                {
                    ++m_row;
                    m_compositor.moveSprite( "Edit", m_row, m_col );
                }
            }
            else if( c == Key::left )
            {
                if( m_col > 0 )
                {
                    --m_col;
                    m_compositor.moveSprite( "Edit", m_row, m_col );
                }
            }
            else if( c == Key::right )
            {
                if( m_col < m_mapTerminal->width() )
                {
                    ++m_col;
                    m_compositor.moveSprite( "Edit", m_row, m_col );
                }
            }
            else if( c == Key::escape )
            {
                m_hotkeys.clear();
                finish();
                m_completed = true;
            }
            else if( c == '\n' )
            {
                m_compositor.spriteToScene( "Edit", m_action, true ); // true = immediate
                m_hotkeys.clear();
                finish();
                m_completed = true;
            }
            else if( c == 'c' )
            {
                ++m_col;
                m_compositor.spriteToScene( "Edit", m_action, true ); // true = immediate
                // FIXME: Could be off screen?
                m_compositor.createSprite( "Edit", m_assetName, m_row, m_col );
            }
            break;
        case updateSelect:
            if( c == Key::escape )
            {
                m_hotkeys.clear();
                finish();
                m_completed = true;
            }
            else if( c == Key::tab )
            {
                m_updateSceneItem = m_compositor.selectNext();
            }
            else if( c == Key::shiftTab )
            {
                m_updateSceneItem = m_compositor.selectPrevious();
            }
            else if( c == Key::up )
            {
                const SceneItem* nextSceneItem( m_compositor.selectDirection( Direction::up ) );
                if( nextSceneItem != NULL )
                {
                    m_updateSceneItem = nextSceneItem;
                }
            }
            else if( c == Key::down )
            {
                const SceneItem* nextSceneItem( m_compositor.selectDirection( Direction::down ) );
                if( nextSceneItem != NULL )
                {
                    m_updateSceneItem = nextSceneItem;
                }
            }
            else if( c == Key::left )
            {
                const SceneItem* nextSceneItem( m_compositor.selectDirection( Direction::left ) );
                if( nextSceneItem != NULL )
                {
                    m_updateSceneItem = nextSceneItem;
                }
            }
            else if( c == Key::right )
            {
                const SceneItem* nextSceneItem( m_compositor.selectDirection( Direction::right ) );
                if( nextSceneItem != NULL )
                {
                    m_updateSceneItem = nextSceneItem;
                }
            }
            else if( c == '\n' )
            {
                m_state = updateForm;
                m_hotkeys.clear();
                drawSceneItemForm();
            }
            else if( c == 'x' )
            {
                m_row = m_updateSceneItem->row();
                m_col = m_updateSceneItem->col();
                m_compositor.deleteCurrentItem( true ); // true = immediate. Also deletes linked programs, texts, attributes.
                m_updateSceneItem = m_compositor.selectClosest( m_row, m_col );
                if( !m_updateSceneItem )
                {
                    m_hotkeys.clear();
                    finish();
                    m_completed = true;
                }
            }
            else if( c == 'm' )
            {
                m_row = m_updateSceneItem->row();
                m_col = m_updateSceneItem->col() + 1; // FIXME: Can go off screen?
                m_compositor.createSprite( "Edit", m_updateSceneItem->assetName(), m_row, m_col );
                drawHotkeysUpdatePlace();
                m_state = updatePlace;
            }
            else if( c == 'c' )
            {
                m_row = m_updateSceneItem->row();
                m_col = m_updateSceneItem->col() + 1; // FIXME: Can go off screen?
                m_assetName = m_updateSceneItem->assetName();
                m_action = m_updateSceneItem->action();
                m_compositor.createSprite( "Edit", m_updateSceneItem->assetName(), m_row, m_col );
                drawHotkeysCreatePlace();
                m_state = createPlace;
            }
            else if( c == 't' )
            {
                if( drawEdit() )
                {
                    m_hotkeys.clear();
                    m_state = editText;
                }
                else
                {
                    drawError( "The text file for this object could not be opened" );
                    m_timer->reset();
                    m_state = editTextError;
                }
            }
            else if( c == 'T' )
            {
                m_compositor.deleteTextCurrentItem();
            }
            else if( c == 'r' )
            {
                String snowflake( m_updateSceneItem->snowflake() );
                m_compositor.raiseCurrentItem( true ); // true = immediate.

                // Select raised/lowered item.
                m_updateSceneItem = m_compositor.selectBy( snowflake );
                if( !m_updateSceneItem )
                {
                    // Shouldn't happen, as item should still exist.
                    m_hotkeys.clear();
                    finish();
                    m_completed = true;
                }
            }
            else if( c == 'l' )
            {
                String snowflake( m_updateSceneItem->snowflake() );
                m_compositor.lowerCurrentItem( true ); // true = immediate.

                // Select raised/lowered item.
                m_updateSceneItem = m_compositor.selectBy( snowflake );
                if( !m_updateSceneItem )
                {
                    // Shouldn't happen, as item should still exist.
                    m_hotkeys.clear();
                    finish();
                    m_completed = true;
                }
            }
            break;
        case updateForm:
            if( c == '\n' )
            {
                if( m_currentForm )
                {
                    m_assetName = m_currentForm->getFieldContents( "Name" );
                    m_action = m_currentForm->getFieldContents( "Action" );

                    AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( m_coordinates, m_assetName ) );
                    if( assetLoader->open() )
                    {
                        delete( assetLoader );
                        
                        closeForm();
                        m_windowManager.setTerminalWindowVisible( m_formWindowName, false );

                        m_compositor.updateCurrentItem( m_assetName, m_action, true ); // true = immediate
                        finish();
                        m_completed = true;
                    }
                    else
                    {
                        delete( assetLoader );
                        drawError( "No such item" );
                        m_timer->reset();
                        m_state = updateFormError;
                    }
                }
            }
            else if( c == Key::escape )
            {
                closeForm();
                m_windowManager.setTerminalWindowVisible( m_formWindowName, false );
                drawHotkeysUpdateSelect();
                m_state = updateSelect;
            }
            else
            {
                if( m_currentForm )
                {
                    m_currentForm->consumeChar( c );
                }
            }
            break;
        case updatePlace:
            if( c == Key::up )
            {
                if( m_row > 0 )
                {
                    --m_row;
                    m_compositor.moveSprite( "Edit", m_row, m_col );
                }
            }
            else if( c == Key::down )
            {
                if( m_row < m_mapTerminal->height() )
                {
                    ++m_row;
                    m_compositor.moveSprite( "Edit", m_row, m_col );
                }
            }
            else if( c == Key::left )
            {
                if( m_col > 0 )
                {
                    --m_col;
                    m_compositor.moveSprite( "Edit", m_row, m_col );
                }
            }
            else if( c == Key::right )
            {
                if( m_col < m_mapTerminal->width() )
                {
                    ++m_col;
                    m_compositor.moveSprite( "Edit", m_row, m_col );
                }
            }
            else if( c == 'm' )
            {
                m_compositor.moveCurrentItem( m_row, m_col, Direction::none, true, true ); // true = moved by keyboard, true = immediate
                m_compositor.deleteSprite( "Edit" );
                m_row = m_updateSceneItem->row();
                m_col = m_updateSceneItem->col();
                m_updateSceneItem = m_compositor.selectClosest( m_row, m_col );
                if( m_updateSceneItem )
                {
                    drawHotkeysUpdateSelect();
                    m_state = updateSelect;
                }
                else
                {
                    m_hotkeys.clear();
                    finish();
                    m_completed = true;
                }
            }
            else if( c == Key::escape )
            {
                m_hotkeys.clear();
                finish();
                m_completed = true;
            }
            else if( c == '\n' )
            {
                m_hotkeys.clear();
                m_compositor.moveCurrentItem( m_row, m_col, Direction::none, true, true ); // true = moved by keyboard, true = immediate
                finish();
                m_completed = true;
            }
            break;
        case editText:
            if( c == control( 's' ) )
            {
                if( closeEditor( true ) )
                {
                    m_windowManager.setTerminalWindowVisible( m_formWindowName, false );
                    m_compositor.addTextCurrentItem();
                    finish();
                    m_completed = true;
                }
                else
                {
                    const char* message( "The item text could not be saved.\
                                          Hit \x1b[97mRet\x1b[0m to go back, then\
                                          \x1b[97mC-S\x1b[0m to try saving again." );
                    drawError( message );
                    m_state = editTextSaveError;
                }
            }
            else if( c == Key::escape )
            {
                if( m_editor && m_editor->modified() )
                {
                    drawDiscard();
                    m_state = editTextDiscard;
                }
                else
                {
                    if( m_editor ) closeEditor( false );
                    m_windowManager.setTerminalWindowVisible( m_formWindowName, false );
                    drawHotkeysUpdateSelect();
                    m_state = updateSelect;
                }
            }
            else
            {
                if( m_editor )
                {
                    m_editor->consumeCharacter( c );
                }
            }
            break;
        case editTextDiscard:
            if( c == 'y' )
            {
                hideDialogue();
                closeEditor( false );
                m_windowManager.setTerminalWindowVisible( m_formWindowName, false );
                drawHotkeysUpdateSelect();
                m_state = updateSelect;
            }
            else
            {
                hideDialogue();
                m_state = editText;
            }
            break;
        case editTextSaveError:
            if( c == Key::newLine )
            {
                hideDialogue();
                m_state = editText;
            }
            break;
        default:
            break;
        }
    }

    if( m_state == createFormError )
    {
        if( m_timer->ms() >= 1000 )
        {
            hideDialogue();
            m_state = createForm;
        }
    }
    else if( m_state == updateFormError )
    {
        if( m_timer->ms() >= 1000 )
        {
            hideDialogue();
            m_state = updateForm;
        }
    }
    else if( m_state == editTextError )
    {
        if( m_timer->ms() >= 1000 )
        {
            hideDialogue();
            m_state = updateSelect;
        }
    }
}

void EditWorld::drawBackground()
{
    AssetLoaders::Baked assetLoader( World::Coordinates(), backgroundAssetName );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        m_formTerminal->consumeNext( 0, 0 );
        m_formTerminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
    }
}

void EditWorld::drawSceneItemForm()
{
    drawBackground();

    Vector< Forms::Field > fields;

    int fieldRow( 0 );
    
    Forms::Field nameField( _Name,
                            textWidth,
                            textHeight,
                            contentFirstRow,
                            contentCol + 7,
                            fieldAttributes,
                            labelAttributes,
                            contentFirstRow,
                            contentCol,
                            SceneItem::maxAssetNameLength() );
    nameField.m_permittedSymbols = "-_";
    if( m_state == updateForm ) nameField.m_contents = m_updateSceneItem->assetName();
    fields.push_back( nameField );
    fieldRow += 2;
    
    if( m_state == updateForm )
    {
        Forms::Field timeField( _Time,
                                textWidth,
                                textHeight,
                                contentFirstRow + fieldRow,
                                contentCol + 7,
                                disabledFieldAttributes,
                                labelAttributes,
                                contentFirstRow + fieldRow,
                                contentCol );
        timeField.m_enabled = false;
        timeField.m_contents = Clock::formatISO( m_updateSceneItem->dateTime() ); // Also converts to VRT.
        fields.push_back( timeField );
        fieldRow += 2;

        String ownerName( m_updateSceneItem->owner() );
        Vector< User >::const_iterator userIt( m_worldMetadata.m_users.begin() );
        for( ; userIt != m_worldMetadata.m_users.end(); ++userIt )
        {
            if( userIt->m_snowflake == ownerName )
            {
                ownerName = userIt->m_name;
                break;
            }
        }

        Forms::Field ownerField( _Owner,
                                textWidth,
                                textHeight,
                                contentFirstRow + fieldRow,
                                contentCol + 7,
                                disabledFieldAttributes,
                                labelAttributes,
                                contentFirstRow + fieldRow,
                                contentCol );
        ownerField.m_enabled = false;
        ownerField.m_contents = ownerName;
        fields.push_back( ownerField );
        fieldRow += 2;
    }

    Forms::Field actionField( _Action,
                              textWidth,
                              textMultiHeight,
                              contentFirstRow + fieldRow,
                              contentCol + 7,
                              fieldAttributes,
                              labelAttributes,
                              contentFirstRow + fieldRow,
                              contentCol,
                              SceneItem::maxActionLength() );
    if( m_state == updateForm ) actionField.m_contents = m_updateSceneItem->action();
    fields.push_back( actionField );

    m_formTerminal->consumeNext( guideRow, contentCol );
    m_formTerminal->consumeString( "\x1b[0;97;100mTab\x1b[37m Select field\x1b[0m  \x1b[97;100mRet\x1b[37m ", false, Terminal::preserveBackground );
    if( m_state == createForm ) m_formTerminal->consumeString( "Create item", false, Terminal::preserveBackground );
    if( m_state == updateForm ) m_formTerminal->consumeString( "Update item", false, Terminal::preserveBackground );
    m_formTerminal->consumeString( "\x1b[0m  \x1b[97;100mEsc\x1b[37m Cancel\x1b[0m", false, Terminal::preserveBackground );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                     m_formWindowName,
                                     String(),
                                     fields,
                                     Forms::Title( m_state == createForm ? _Insert_Item : _Update_Item,
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

bool EditWorld::drawEdit()
{
    Coordinates coordinates( m_worldMetadata.m_worldID );
    m_editor = m_editorFactory.makeEditor( coordinates,
                                           m_updateSceneItem->snowflake() + "_txt",
                                           String(),
                                           String(),
                                           m_updateSceneItem->snowflake(),
                                           "\x1b[0m\x1b[97;100mC-S\x1b[37m Save\x1b[0m  \x1b[97;100mEsc\x1b[37m Discard\x1b[0m" );
    if( m_editor->open( Agape::Editor::Editor::modeWrite | Agape::Editor::Editor::modeCreate ) )
    {
        m_windowManager.setTerminalWindowVisible( m_formWindowName, true );
        m_editor->draw();
        return true;
    }

    return false;
}

void EditWorld::drawDiscard()
{
    const char* message( "Are you sure you want to discard this text?\
                          Hit \x1b[97mY\x1b[0m to confirm, or\
                          any other key to go back." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Are you sure?" );
    m_dialogue.drawMessage( message );
}

void EditWorld::drawError( const String& message )
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void EditWorld::hideDialogue()
{
    m_dialogue.hide();
}

void EditWorld::drawHotkeysCreatePlace()
{
    m_hotkeys.clear();
    m_hotkeys.show( "\x18\x19\x1B\x1A", "Move" );
    m_hotkeys.show( "Ret", "Place" );
    m_hotkeys.show( "C", "lone" );
    m_hotkeys.show( "Esc", "Cancel" );
}

void EditWorld::drawHotkeysUpdateSelect()
{
    m_hotkeys.clear();
    m_hotkeys.show( "\x18\x19\x1B\x1A", "Select" );
    m_hotkeys.show( "Tab", "Next" );
    m_hotkeys.show( "Sh-Tab", "Prev" );
    m_hotkeys.spacer();
    m_hotkeys.show( "Ret", "Update" );
    m_hotkeys.spacer();
    m_hotkeys.show( "X", "Delete" );
    m_hotkeys.show( "M", "ove" );
    m_hotkeys.show( "C", "lone" );
    m_hotkeys.spacer();
    m_hotkeys.show( "T", "ext" );
    m_hotkeys.show( "Sh-T", "DelTxt" );
    m_hotkeys.spacer();
    m_hotkeys.show( "R", "aise" );
    m_hotkeys.show( "L", "ower" );
    m_hotkeys.spacer();
    m_hotkeys.show( "Esc", "Cancel" );
}

void EditWorld::drawHotkeysUpdatePlace()
{
    m_hotkeys.clear();
    m_hotkeys.show( "\x18\x19\x1B\x1A", "Move" );
    m_hotkeys.show( "Ret", "Place" );
    m_hotkeys.show( "M", "Sel next" );
    m_hotkeys.show( "Esc", "Cancel" );
}

bool EditWorld::closeEditor( bool save )
{
    if( m_editor->close( save ) )
    {
        delete( m_editor );
        m_editor = nullptr;
        return true;
    }

    return false;
}

void EditWorld::closeForm()
{
    if( m_currentForm )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

void EditWorld::start()
{
    m_compositor.lockScene( true );
}

void EditWorld::finish()
{
    m_compositor.deleteSprite( "Edit" );
    m_compositor.selectNone();
    m_compositor.lockScene( false );
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
