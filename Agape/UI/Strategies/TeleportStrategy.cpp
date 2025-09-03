#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "Assets/Asset.h"
#include "Encryptors/Factories/EncryptorsFactory.h"
#include "Encryptors/Encryptor.h"
#include "InputDevices/InputDevice.h"
#include "PresenceLoaders/Factories/PresenceLoadersFactory.h"
#include "PresenceLoaders/PresenceLoader.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "UI/Forms/Form.h"
#include "UI/Dialogue.h"
#include "Utils/LiteStream.h"
#include "Utils/Snowflake.h"
#include "World/ScenePresence.h"
#include "World/Teleport.h"
#include "World/User.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "WorldLoaders/Factories/WorldLoadersFactory.h"
#include "WorldLoaders/WorldLoader.h"
#include "ANSITerminal.h"
#include "Collections.h"
#include "ConfigurationStore.h"
#include "String.h"
#include "StringConstants.h"
#include "TeleportStrategy.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"
#include "Worldbook.h"

#include <stdlib.h>

#include <algorithm>
#include <cmath>

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

    const int selectWidth( 51 );
    const int selectHeight( 5 );
    const int fieldAttributes( 0x07 );
    const int selectionAttributes( 0x4F );
    
    const int textWidth( 51 );
    const int textHeight( 1 );
    const int textNarrowWidth( 10 );

    const int guideRow( 15 );

    const int worldNameMaxWidth( 15 );
    const int coordsMaxWidth( 19 );

    const String backgroundAssetName( "large-dialogue" );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

Teleport::Teleport( InputDevice& inputDevice,
                    WorldLoaders::Factory& worldLoaderFactory,
                    PresenceLoaders::Factory& presenceLoaderFactory,
                    const User& worldUser,
                    const Metadata& worldMetadata,
                    const Coordinates& coordinates,
                    const Worldbook& worldbook,
                    ConfigurationStore& configurationStore,
                    Encryptors::Factory& encryptorFactory,
                    Snowflake& snowflake,
                    WindowManager& windowManager,
                    const String& windowName,
                    Dialogue& dialogue,
                    Timers::Factory& timerFactory ) :
  m_inputDevice( inputDevice ),
  m_worldLoaderFactory( worldLoaderFactory ),
  m_presenceLoaderFactory( presenceLoaderFactory ),
  m_worldUser( worldUser ),
  m_worldMetadata( worldMetadata ),
  m_coordinates( coordinates ),
  m_worldbook( worldbook ),
  m_configurationStore( configurationStore ),
  m_snowflake( snowflake ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_dialogue( dialogue ),
  m_encryptor( encryptorFactory.makeEncryptor() ),
  m_currentRow( 0 ),
  m_currentCol( 0 ),
  m_state( none ),
  m_previousState( none ),
  m_completed( false ),
  m_timer( timerFactory.makeTimer() ),
  m_terminal( nullptr ),
  m_currentForm( nullptr ),
  m_worldLoader( worldLoaderFactory.makeLoader() )
{
}

Teleport::~Teleport()
{
    closeForm();
    delete( m_encryptor );
    delete( m_timer );
    delete( m_worldLoader );
}

void Teleport::enter( const Value& parameters )
{
    m_currentRow = parameters[_row];
    m_currentCol = parameters[_column];

    m_completed = false;
    m_returnParameters = Value();

    if( !parameters.hasValue( _teleport ) ) // Standard UI mode.
    {
        WindowManager::TerminalWindow terminalWindow;
        if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
        {
            m_terminal = terminalWindow.m_terminal;
            if( drawList() )
            {
                m_state = list;
            }
            else
            {
                drawListError();
                m_timer->reset();
                m_state = listError;
            }
        }
        else
        {
            m_completed = true; // Uh oh!
        }
    }
    else // Non-UI add or add-and-immediate-go.
    {
        m_newTeleport = World::Teleport::fromValue( parameters[_teleport] );
        m_newTeleport.m_snowflake = m_snowflake.generate();
        
        drawAddPending();
        if( addTeleport() )
        {
            if( (int)parameters[_home] == 1 )
            {
                m_newTeleport.toValue( m_configurationStore.get( _home ) );
                m_configurationStore.save();
            }
            
            hideDialogue();
            
            if( (int)parameters[_doTeleport] == 1 )
            {
                doTeleport( m_newTeleport );
            }
            
            m_returnParameters[_success] = 1;
            m_completed = true;
        }
        else
        {
            hideDialogue();
            m_returnParameters[_success] = 0;
            m_completed = true;
        }
    }
}

void Teleport::returnTo( const Value& parameters )
{
}

bool Teleport::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool Teleport::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        closeForm();
        
        m_windowManager.setTerminalWindowVisible( m_windowName, false );

        parameters = m_returnParameters;

        return true;
    }

    return false;
}

void Teleport::run()
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
        case list:
            if( ( c == Key::newLine ) )
            {
                if( m_currentForm )
                {
                    const Forms::Field& currentField( m_currentForm->currentField() );
                    if( currentField.m_name == _Teleports )
                    {
                        if( doTeleportCurrent() )
                        {
                            m_completed = true;
                        }
                        else
                        {
                            drawGoError();
                            m_timer->reset();
                            m_state = goError;
                        }
                    }
                    else if( currentField.m_name == _People )
                    {
                        if( doTeleportPersonCurrent() )
                        {
                            m_completed = true;
                        }
                        else
                        {
                            drawGoError();
                            m_timer->reset();
                            m_state = goError;
                        }
                    }
                }
            }
            else if( c == 'a' )
            {
                if( drawAdd( false ) )
                {
                    m_state = add;
                }
                else
                {
                    drawAddError();
                    m_timer->reset();
                    m_state = addError;
                }
            }
            else if( c == 'h' )
            {
                if( drawAdd( true ) )
                {
                    m_state = add;
                }
                else
                {
                    drawAddError();
                    m_timer->reset();
                    m_state = addError;
                }
            }
            else if( c == 'x' )
            {
                if( m_currentForm )
                {
                    const Forms::Field& currentField( m_currentForm->currentField() );
                    if( currentField.m_name == _Teleports )
                    {
                        drawDeletePending();
                        if( deleteTeleport() )
                        {
                            drawDeleteSuccess();
                            m_timer->reset();
                            m_state = deleteSuccess;
                        }
                        else
                        {
                            drawDeleteError();
                            m_timer->reset();
                            m_state = deleteError;
                        }
                    }
                }
            }
            else if( c == 'g' )
            {
                drawGoTo();
                m_state = goTo;
            }
            else if( c == control( 'h' ) )
            {
                if( m_currentForm )
                {
                    const Forms::Field& currentField( m_currentForm->currentField() );
                    if( currentField.m_name == _Teleports )
                    {
                        drawSetHomePending();
                        if( setHome() )
                        {
                            drawSetHomeSuccess();
                            m_timer->reset();
                            m_state = setHomeSuccess;
                        }
                        else
                        {
                            drawSetHomeError();
                            m_timer->reset();
                            m_state = setHomeError;
                        }
                    }
                }
            }
            else if( c == Key::escape )
            {
                m_completed = true;
            }
            else if( m_currentForm )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        case add:
            if( c == Key::newLine )
            {
                if( getTeleport() )
                {
                    drawAddPending();
                    if( addTeleport() )
                    {
                        drawAddSuccess();
                        m_timer->reset();
                        m_state = addSuccess;
                    }
                    else
                    {
                        drawAddError();
                        m_timer->reset();
                        m_state = addError;
                    }
                }
                else
                {
                    drawSyntaxError();
                    m_timer->reset();
                    m_previousState = m_state;
                    m_state = syntaxError;
                }
            }
            else if( c == Key::escape )
            {
                drawList();
                m_state = list;
            }
            else if( m_currentForm )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        case goTo:
            if( c == Key::newLine )
            {
                if( getTeleport() )
                {
                    if( doTeleportGoTo() )
                    {
                        m_completed = true;
                    }
                    else
                    {
                        drawGoError();
                        m_timer->reset();
                        m_state = goError;
                    }
                }
                else
                {
                    drawSyntaxError();
                    m_timer->reset();
                    m_previousState = m_state;
                    m_state = syntaxError;
                }
            }
            else if( c == Key::escape )
            {
                drawList();
                m_state = list;
            }
            else if( m_currentForm )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        default:
            break;
        }
    }

    if( m_state == listError )
    {
        if( m_timer->ms() >= 2000 )
        {
            hideDialogue();
            m_completed = true;
        }
    }
    else if( m_state == syntaxError )
    {
        if( m_timer->ms() >= 2000 )
        {
            hideDialogue();
            m_state = m_previousState; // Either add or goTo.
        }
    }
    else if( m_state == addError )
    {
        if( m_timer->ms() >= 2000 )
        {
            hideDialogue();
            m_state = add;
        }
    }
    else if( ( m_state == goError ) ||
             ( m_state == addSuccess ) ||
             ( m_state == deleteSuccess ) ||
             ( m_state == deleteError ) ||
             ( m_state == setHomeSuccess ) ||
             ( m_state == setHomeError ) )
    {
        if( m_timer->ms() >= 2000 )
        {
            hideDialogue();
            drawList();
            m_state = list;
        }
    }
}

void Teleport::drawBackground()
{
    AssetLoaders::Baked assetLoader( World::Coordinates(), backgroundAssetName );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        m_terminal->consumeNext( 0, 0 );
        m_terminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
    }
}

void Teleport::drawLoadPending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Loading" );
    m_dialogue.drawMessage( "Loading..." );
}

bool Teleport::drawList()
{
    drawLoadPending();

    bool success( true );

    World::Teleport home;
    if( m_configurationStore.hasKey( _home ) )
    {
        home = World::Teleport::fromValue( m_configurationStore.get( _home ) );
    }

    Vector< Forms::Field > fields;

    {
    m_teleports.clear();
    String reason; // FIXME: Pass back to error dialogue?
#ifdef __EMSCRIPTEN__
    bool allDevices( true );
#else
    bool allDevices( false );
#endif
    if( m_worldLoader && m_worldLoader->loadTeleports( m_teleports, allDevices, reason ) )
    {
        // Create place teleports select list.
        LiteStream entryStream;

        std::sort( m_teleports.begin(), m_teleports.end(), World::Teleport::newer );

        bool first( true );
        Vector< World::Teleport >::iterator teleportIt( m_teleports.begin() );
        for( ; teleportIt != m_teleports.end(); ++teleportIt )
        {
            if( !first )
            {
                entryStream << "\x1F"; // Delimiter
            }
            first = false;

            World::Teleport& thisTeleport( *teleportIt );

            World::Metadata metadata;
            bool haveWorldMetadata( m_worldbook.getMetadata( thisTeleport.m_coordinates.m_worldID, metadata ) );
            if( haveWorldMetadata )
            {
                m_encryptor->setKey( metadata.m_itemKey.c_str() );
                thisTeleport.decrypt( *m_encryptor );
            }

            String description( thisTeleport.m_name );
            bool isHome( thisTeleport == home );
            String shortDescription;
            int descriptionMaxWidth( selectWidth - worldNameMaxWidth - coordsMaxWidth - 1 );
            if( isHome ) descriptionMaxWidth -= 2; // Make space for home marker.
            if( haveWorldMetadata )
            {
                if( description.length() <= descriptionMaxWidth )
                {
                    shortDescription = description;
                    for( int i = 0; i < ( descriptionMaxWidth - description.length() ); ++i ) shortDescription += " ";
                }
                else
                {
                    shortDescription = description.substr( 0, descriptionMaxWidth - 3 );
                    shortDescription += "...";
                }
            }
            else
            {
                shortDescription = "???";
                for( int i = 0; i < ( descriptionMaxWidth - 3 ); ++i ) shortDescription += " ";
            }

            if( isHome ) // Add home marker
            {
                shortDescription = ANSITerminal::colour( Terminal::colBrightMagenta ) +
                                   "H " +
                                   ANSITerminal::colour( Terminal::colWhite ) +
                                   shortDescription;
            }
            else
            {
                shortDescription = ANSITerminal::colour( Terminal::colWhite ) +
                                   shortDescription;
            }

            entryStream << shortDescription << ANSITerminal::colour( Terminal::colGrey ) << "@";

            String worldName;
            if( haveWorldMetadata )
            {
                worldName = metadata.m_name;
            }
            else
            {
                worldName = "???";
            }

            String shortWorldName;
            if( worldName.length() <= worldNameMaxWidth - 1 )
            {
                shortWorldName = worldName;
                for( int i = 0; i < ( worldNameMaxWidth - worldName.length() - 1 ); ++i ) shortWorldName += " ";
            }
            else
            {
                shortWorldName = worldName.substr( 0, worldNameMaxWidth - 4 );
                shortWorldName += "...";
            }

            entryStream << shortWorldName << " ";


            entryStream << std::abs( thisTeleport.m_coordinates.m_y );
            if( thisTeleport.m_coordinates.m_y >= 0 )
            {
                entryStream << "N ";
            }
            else
            {
                entryStream << "S ";
            }

            entryStream << std::abs( thisTeleport.m_coordinates.m_x );
            if( thisTeleport.m_coordinates.m_x >= 0 )
            {
                entryStream << "E ";
            }
            else
            {
                entryStream << "W ";
            }

            entryStream << thisTeleport.m_row << "," << thisTeleport.m_col;
        }

        Forms::Field teleportsField( _Teleports,
                                     selectWidth,
                                     selectHeight,
                                     contentFirstRow,
                                     contentCol,
                                     fieldAttributes,
                                     entryStream.str(),
                                     selectionAttributes,
                                     0, // Label attributes
                                     '\x1F' ); // Delim
        teleportsField.m_enabled = !m_teleports.empty();
        fields.push_back( teleportsField );
    }
    else
    {
        success = false;
    }
    }

    {
    // Create people teleports select list.
    if( success )
    {
        PresenceLoader* presenceLoader( m_presenceLoaderFactory.makeLoader( m_coordinates, PresenceLoader::noReceiveRequests ) );
        success = presenceLoader->loadWorld( m_worldPresences );
        delete( presenceLoader );
    }

    if( success )
    {
        LiteStream entryStream;

        {
        Vector< ScenePresence >::const_iterator presenceIt( m_worldPresences.begin() );
        for( ; presenceIt != m_worldPresences.end(); ++presenceIt )
        {
            if( presenceIt->m_user == m_worldUser )
            {
                m_worldPresences.erase( presenceIt ); // Don't show ourselves
                break;
            }
        }
        std::sort( m_worldPresences.begin(), m_worldPresences.end(), ScenePresence::newer );
        }

        bool first( true );
        Vector< ScenePresence >::const_iterator presenceIt( m_worldPresences.begin() );
        for( ; presenceIt != m_worldPresences.end(); ++presenceIt )
        {
            ScenePresence presence( *presenceIt );

            if( !first )
            {
                entryStream << "\x1F"; // Delimiter
            }
            first = false;

            // Glyph and name
            entryStream << ANSITerminal::colour( presence.m_user.m_attributes )
                        << ANSITerminal::charset( Terminal::avatarCharset )
                        << String( 1, (char)presence.m_user.m_glyph )
                        << ANSITerminal::resetCharset()
                        << ANSITerminal::colour( ANSITerminal::colWhite )
                        << " ";

            int nameMaxWidth( selectWidth - coordsMaxWidth - 3 );
            String shortUserName( nameMaxWidth, ' ' );
            const String& userName( presence.m_user.m_name );
            if( userName.length() > nameMaxWidth - 3 )
            {
                shortUserName = userName.substr( 0, nameMaxWidth - 3 );
                shortUserName += "...";
            }
            else
            {
                shortUserName.replace( 0, userName.length(), userName );
            }
            entryStream << shortUserName << " " << ANSITerminal::colour( Terminal::colGrey );

            // Last coordinates
            entryStream << std::abs( presence.m_coordinates.m_y );
            if( presence.m_coordinates.m_y >= 0 )
            {
                entryStream << "N ";
            }
            else
            {
                entryStream << "S ";
            }

            entryStream << std::abs( presence.m_coordinates.m_x );
            if( presence.m_coordinates.m_x >= 0 )
            {
                entryStream << "E ";
            }
            else
            {
                entryStream << "W ";
            }

            entryStream << presence.m_row << "," << presence.m_col;
        }

        Forms::Field presencesField( _People,
                                     selectWidth,
                                     selectHeight,
                                     contentFirstRow + selectHeight + 1,
                                     contentCol,
                                     fieldAttributes,
                                     entryStream.str(),
                                     selectionAttributes,
                                     0, // Label attributes
                                     '\x1F' ); // Delim
        presencesField.m_enabled = !m_worldPresences.empty();
        fields.push_back( presencesField );
    }
    }

    hideDialogue();

    if( success )
    {
        drawBackground();

        m_terminal->consumeNext( guideRow - 1, contentCol );
        m_terminal->consumeString( "\x1b[0;97;100mTab\x1b[37m Switch\x1b[0m      \x1b[0;97;100m\x18/\x19\x1b[37m Select\x1b[0m  \x1b[97;100mRet\x1b[37m Go\x1b[0m   \x1b[97;100mEsc\x1b[37m Exit\x1b[0m\r\n", Terminal::scrollLock, Terminal::preserveBackground );
        m_terminal->consumeNext( guideRow, contentCol );
        m_terminal->consumeString( "\x1b[97;100mA\x1b[37m Add\x1b[0m  \x1b[97;100mH\x1b[37m Here\x1b[0m   \x1b[97;100mX\x1b[37m Delete\x1b[0m    \x1b[97;100mG\x1b[37m Go to\x1b[0m  \x1b[97;100mC-H\x1b[37m Tgl home\x1b[0m", Terminal::scrollLock, Terminal::preserveBackground );

        m_windowManager.setTerminalWindowVisible( m_windowName, true );

        closeForm();

        m_currentForm = new Forms::Form( m_windowManager,
                                         m_windowName,
                                         String(),
                                         fields,
                                         Forms::Title( _Teleports,
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
    }

    return success;
}

void Teleport::drawListError()
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( "Sorry, your teleports could not be loaded at this time." );
}

void Teleport::drawGoError()
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( "An error was encountered while teleporting. Hopefully all your molecules are still in one place." );
}

bool Teleport::drawAdd( bool here )
{
    drawLoadPending();

    drawBackground();

    bool success( true );

    Vector< Forms::Field > fields;

    Vector< World::Metadata > joinedWorlds;
    String reason;
    if( m_worldLoader->loadJoinedWorlds( joinedWorlds, false, reason ) ) // false = only this device.
    {
        m_worldListIDs.clear();

        LiteStream entryStream;

        // Add the current world to the top of the list.
        if( m_worldMetadata.m_name.length() > selectWidth - 3 )
        {
            entryStream << m_worldMetadata.m_name.substr( 0, selectWidth - 3 )
                        << "...";
        }
        else
        {
            if( !m_worldMetadata.m_name.empty() )
            {
                entryStream << m_worldMetadata.m_name;
            }
            else
            {
                entryStream << "???";
            }
        }
        m_worldListIDs.push_back( m_worldMetadata.m_worldID );

        // Add other joined worlds.
        Vector< World::Metadata >::const_iterator it( joinedWorlds.begin() );
        for( ; it != joinedWorlds.end(); ++it )
        {
            World::Metadata metadata;
            if( ( it->m_worldID != m_worldMetadata.m_worldID ) &&
                  m_worldbook.getMetadata( it->m_worldID, metadata ) )
            {
                entryStream << "\x1F"; // Delimiter

                const String& worldName( metadata.m_name );

                if( worldName.length() > selectWidth - 3 )
                {
                    entryStream << worldName.substr( 0, selectWidth - 3 )
                                << "...";
                }
                else
                {
                    if( !worldName.empty() )
                    {
                        entryStream << worldName;
                    }
                    else
                    {
                        entryStream << "???";
                    }
                }
                m_worldListIDs.push_back( it->m_worldID );
            }
        }

        fields.push_back( Forms::Field( _World,
                                        selectWidth,
                                        selectHeight,
                                        contentFirstRow,
                                        contentCol,
                                        fieldAttributes,
                                        entryStream.str(),
                                        selectionAttributes,
                                        labelAttributes,
                                        '\x1F' ) ); // Delim

        Forms::Field yfield( _Y,
                            textNarrowWidth,
                            textHeight,
                            contentFirstRow + selectHeight + 1,
                            contentCol + 14,
                            fieldAttributes,
                            labelAttributes );
        yfield.m_labelRow = contentFirstRow + selectHeight + 2;
        yfield.m_labelCol = contentCol;

        Forms::Field xfield( _X,
                            textNarrowWidth,
                            textHeight,
                            contentFirstRow + selectHeight + 1,
                            contentCol + textNarrowWidth + 31,
                            fieldAttributes,
                            labelAttributes );
        xfield.m_labelRow = contentFirstRow + selectHeight + 2;
        xfield.m_labelCol = contentCol + textNarrowWidth + 17;

        Forms::Field rowfield( _Row,
                                textNarrowWidth,
                                textHeight,
                                contentFirstRow + selectHeight + 3,
                                contentCol + 14,
                                fieldAttributes,
                                labelAttributes );
        rowfield.m_labelRow = contentFirstRow + selectHeight + 4;
        rowfield.m_labelCol = contentCol;

        Forms::Field colfield( _Col,
                                textNarrowWidth,
                                textHeight,
                                contentFirstRow + selectHeight + 3,
                                contentCol + textNarrowWidth + 31,
                                fieldAttributes,
                                labelAttributes );
        colfield.m_labelRow = contentFirstRow + selectHeight + 4;
        colfield.m_labelCol = contentCol + textNarrowWidth + 17;

        Forms::Field descfield( _Description,
                                37,
                                textHeight,
                                contentFirstRow + selectHeight + 5,
                                contentCol + 14,
                                fieldAttributes,
                                labelAttributes );
        descfield.m_labelRow = contentFirstRow + selectHeight + 6;
        descfield.m_labelCol = contentCol + 2;

        fields.push_back( yfield );
        fields.push_back( xfield );
        fields.push_back( rowfield );
        fields.push_back( colfield );
        fields.push_back( descfield );

        m_terminal->consumeNext( guideRow, contentCol );
        m_terminal->consumeString( "\x1b[97;100mTab\x1b[37m Next\x1b[0m  \x1b[97;100mRet\x1b[37m Continue\x1b[0m  \x1b[97;100mEsc\x1b[37m Go back\x1b[0m", Terminal::scrollLock, Terminal::preserveBackground );

        closeForm();

        m_currentForm = new Forms::Form( m_windowManager,
                                        m_windowName,
                                        String(),
                                        fields,
                                        Forms::Title( _Add_Teleport,
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

            if( here ) setHere();
        }
    }
    else
    {
        success = false;
    }

    hideDialogue();

    return success;
}

void Teleport::drawAddPending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Adding" );
    m_dialogue.drawMessage( "Adding teleport \"" + m_newTeleport.m_name + "\". Please wait." );
}

void Teleport::drawAddSuccess()
{
    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( "Teleport added successfully." );
}

void Teleport::drawSyntaxError()
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Syntax Error" );
    m_dialogue.drawMessage( "'Syntax error' what can it be? 'Syntax error' press the wrong key" );
}

void Teleport::drawAddError()
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( "Sorry, we were unable to add your teleport at this time." );
}

void Teleport::drawGoTo()
{
    drawBackground();

    Vector< Forms::Field > fields;

    Forms::Field yfield( _Y,
                         textNarrowWidth,
                         textHeight,
                         contentFirstRow + selectHeight + 1,
                         contentCol + 14,
                         fieldAttributes,
                         labelAttributes );
    yfield.m_labelRow = contentFirstRow + selectHeight + 2;
    yfield.m_labelCol = contentCol;

    Forms::Field xfield( _X,
                         textNarrowWidth,
                         textHeight,
                         contentFirstRow + selectHeight + 1,
                         contentCol + textNarrowWidth + 31,
                         fieldAttributes,
                         labelAttributes );
    xfield.m_labelRow = contentFirstRow + selectHeight + 2;
    xfield.m_labelCol = contentCol + textNarrowWidth + 17;

    Forms::Field rowfield( _Row,
                            textNarrowWidth,
                            textHeight,
                            contentFirstRow + selectHeight + 3,
                            contentCol + 14,
                            fieldAttributes,
                            labelAttributes );
    rowfield.m_labelRow = contentFirstRow + selectHeight + 4;
    rowfield.m_labelCol = contentCol;

    Forms::Field colfield( _Col,
                            textNarrowWidth,
                            textHeight,
                            contentFirstRow + selectHeight + 3,
                            contentCol + textNarrowWidth + 31,
                            fieldAttributes,
                            labelAttributes );
    colfield.m_labelRow = contentFirstRow + selectHeight + 4;
    colfield.m_labelCol = contentCol + textNarrowWidth + 17;

    fields.push_back( yfield );
    fields.push_back( xfield );
    fields.push_back( rowfield );
    fields.push_back( colfield );

    m_terminal->consumeNext( guideRow, contentCol );
    m_terminal->consumeString( "\x1b[97;100mTab\x1b[37m Next\x1b[0m  \x1b[97;100mRet\x1b[37m Continue\x1b[0m  \x1b[97;100mEsc\x1b[37m Go back\x1b[0m", Terminal::scrollLock, Terminal::preserveBackground );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                    m_windowName,
                                    String(),
                                    fields,
                                    Forms::Title( _Go_To,
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
}

void Teleport::drawDeletePending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Deleting" );
    m_dialogue.drawMessage( "Deleting teleport. Please wait." );
}

void Teleport::drawDeleteSuccess()
{
    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( "Teleport deleted successfully." );
}

void Teleport::drawDeleteError()
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( "Sorry, we were unable to delete that teleport." );
}

void Teleport::drawSetHomePending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Setting Home" );
    m_dialogue.drawMessage( "Setting home. Please wait." );
}

void Teleport::drawSetHomeSuccess()
{
    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( "Home location set successfully." );
}

void Teleport::drawSetHomeError()
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( "Sorry, we were unable to set your home position." );
}

void Teleport::hideDialogue()
{
    m_dialogue.hide();
}

void Teleport::closeForm()
{
    if( m_currentForm )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

void Teleport::setHere()
{
    if( m_currentForm )
    {
        // Y
        {
        LiteStream stream;
        if( m_coordinates.m_y >= 0 )
        {
            stream << m_coordinates.m_y << "N";
        }
        else
        {
            stream << ( m_coordinates.m_y * -1 ) << "S";
        }
        String yStr( stream.str() );
        m_currentForm->setFieldContents( _Y, yStr );
        }

        // X
        {
        LiteStream stream;
        if( m_coordinates.m_x >= 0 )
        {
            stream << m_coordinates.m_x << "E";
        }
        else
        {
            stream << ( m_coordinates.m_x * -1 ) << "W";
        }
        String xStr( stream.str() );
        m_currentForm->setFieldContents( _X, xStr );
        }

        // Row
        {
        LiteStream stream;
        stream << m_currentRow;
        String rowStr( stream.str() );
        m_currentForm->setFieldContents( _Row, rowStr );
        }

        // Col
        {
        LiteStream stream;
        stream << m_currentCol;
        String colStr( stream.str() );
        m_currentForm->setFieldContents( _Col, colStr );
        }

        m_currentForm->setCurrentField( _Description );

        m_currentForm->draw();
    }
}

bool Teleport::getTeleport()
{
    if( m_currentForm )
    {
        m_newTeleport.m_snowflake = m_snowflake.generate();

        int idx( m_currentForm->getSelectIndex( _World ) );
        if( idx >= 0 )
        {
            m_newTeleport.m_coordinates.m_worldID = m_worldListIDs[idx];
        }
        else
        {
            if( m_state == add )
            {
                return false;
            }
            else // goTo
            {
                m_newTeleport.m_coordinates.m_worldID = m_coordinates.m_worldID;
            }
        }

        String xstr( m_currentForm->getFieldContents( _X ) );
        String ystr( m_currentForm->getFieldContents( _Y ) );

        if( xstr.empty() || ystr.empty() ||
            !( ( xstr[0] >= '0' ) && ( xstr[0] <= '9' ) &&
               ( ystr[0] >= '0' ) && ( ystr[0] <= '9' ) ) )
        {
            return false;
        }
        
        // FIXME: Undefined behaviour if input invalid/out of range?
        m_newTeleport.m_coordinates.m_x = ::atoi( xstr.c_str() ); // Will ignore trailing non-digits.
        m_newTeleport.m_coordinates.m_y = ::atoi( ystr.c_str() );

        if( xstr.find( 'W' ) != String::npos ) m_newTeleport.m_coordinates.m_x *= -1;
        if( ystr.find( 'S' ) != String::npos ) m_newTeleport.m_coordinates.m_y *= -1;

        String rowstr( m_currentForm->getFieldContents( _Row ) );
        String colstr( m_currentForm->getFieldContents( _Col ) );

        if( !rowstr.empty() &&
            !( ( rowstr[0] >= '0' ) && ( rowstr[0] <= '9' ) ) )
        {
            return false;
        }

        if( !colstr.empty() &&
            !( ( colstr[0] >= '0' ) && ( colstr[0] <= '9' ) ) )
        {
            return false;
        }

        m_newTeleport.m_row = rowstr.empty() ? m_currentRow : ::atoi( rowstr.c_str() );
        m_newTeleport.m_col = colstr.empty() ? m_currentCol : ::atoi( colstr.c_str() );

        m_newTeleport.m_name = m_currentForm->getFieldContents( _Description );

        if( ( m_state == add ) && m_newTeleport.m_name.empty() )
        {
            return false;
        }

        return true;
    }

    return false;
}

bool Teleport::doTeleportCurrent()
{
    int idx( m_currentForm->getSelectIndex( _Teleports ) );
    if( idx >= 0 )
    {
        return doTeleport( m_teleports[idx] );
    }

    return false;
}

bool Teleport::doTeleportPersonCurrent()
{
    int idx( m_currentForm->getSelectIndex( _People ) );
    if( idx >= 0 )
    {
        World::Teleport teleport;
        teleport.m_coordinates.m_worldID = m_worldMetadata.m_worldID;
        const World::ScenePresence& scenePresence( m_worldPresences[idx] );
        teleport.m_coordinates.m_x = scenePresence.m_coordinates.m_x;
        teleport.m_coordinates.m_y = scenePresence.m_coordinates.m_y;
        teleport.m_row = scenePresence.m_row;
        teleport.m_col = ( scenePresence.m_col > 0 ) ? scenePresence.m_col - 1 : scenePresence.m_col + 1;

        return doTeleport( teleport );
    }

    return false;
}

bool Teleport::doTeleportGoTo()
{
    return doTeleport( m_newTeleport );
}

bool Teleport::doTeleport( const World::Teleport& teleport )
{
    m_returnParameters[_doTeleport] = 1;
    if( teleport.m_coordinates.m_worldID != m_worldMetadata.m_worldID )
    {
        m_returnParameters[_reenter] = 1; // Need to re-enter to go to new world.
    }
    teleport.m_coordinates.toValue( m_returnParameters[_coordinates] );
    m_returnParameters[_row] = teleport.m_row;
    m_returnParameters[_column] = teleport.m_col;

    return true;
}

bool Teleport::addTeleport()
{
    // FIXME: Loading assets and scenes, we offload responsibility for encrypt/
    // decrypt to a loader interposed between the user of the loader and the
    // actual loader, to make this transparent and DRY. We should probably do
    // that for WorldLoaders too, for teleport load/create functions.

    // TODO: Show reason?
    String reason;
    Metadata metadata;
    bool haveWorldMetadata( m_worldbook.getMetadata( m_newTeleport.m_coordinates.m_worldID, metadata ) );
    if( haveWorldMetadata )
    {
        m_encryptor->setKey( metadata.m_itemKey.c_str() );
        m_newTeleport.encrypt( *m_encryptor );
        return( m_worldLoader && m_worldLoader->createTeleport( m_newTeleport, reason ) );
    }

    return false;
}

bool Teleport::deleteTeleport()
{
    if( m_currentForm )
    {
        int idx( m_currentForm->getSelectIndex( _Teleports ) );
        if( idx >= 0 )
        {
            World::Teleport& teleport( m_teleports[idx] );
            // TODO: Show reason?
            String reason;
            return( m_worldLoader && m_worldLoader->deleteTeleport( teleport, reason ) );
        }
    }

    return false;
}

bool Teleport::setHome()
{
    if( m_currentForm )
    {
        int idx( m_currentForm->getSelectIndex( _Teleports ) );
        if( idx >= 0 )
        {
            World::Teleport& currentTeleport( m_teleports[idx] );
            World::Teleport homeTeleport( World::Teleport::fromValue( m_configurationStore.get( _home ) ) );
            if( currentTeleport.m_snowflake != homeTeleport.m_snowflake )
            {
                currentTeleport.toValue( m_configurationStore.get( _home ) );
            }
            else
            {
                m_configurationStore.remove( _home );
            }
            m_configurationStore.save();
            return true;
        }
    }

    return false;
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
