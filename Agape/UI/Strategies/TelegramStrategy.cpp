#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "Assets/Asset.h"
#include "Clocks/Clock.h"
#include "Encryptors/Factories/EncryptorsFactory.h"
#include "Encryptors/Encryptor.h"
#include "InputDevices/InputDevice.h"
#include "TelegramLoaders/Factories/TelegramLoadersFactory.h"
#include "TelegramLoaders/TelegramLoader.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "UI/Forms/Form.h"
#include "UI/Forms/FormTypes.h"
#include "UI/Dialogue.h"
#include "Utils/LiteStream.h"
#include "Utils/Snowflake.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "World/Telegram.h"
#include "World/User.h"
#include "ANSITerminal.h"
#include "Collections.h"
#include "Editor.h"
#include "EditorFactory.h"
#include "String.h"
#include "StringConstants.h"
#include "TelegramStrategy.h"
#include "Terminal.h"
#include "Value.h"
#include "WindowManager.h"
#include "Worldbook.h"

#include <algorithm>

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
    const int selectHeight( 10 );
    const int fieldAttributes( 0x07 );
    const int selectionAttributes( 0x4F );
    
    const int textWidth( 51 );
    const int textHeight( 1 );

    const int worldsRow( 12 );
    const int guideRow( 14 );

    const String backgroundAssetName( "large-dialogue" );

    const char defaultSenderGlyph( '\x80' );
    const int defaultSenderAttributes( 0x07 );

    const int unreadAttributes( 0x0F );
    const int readAttributes( 0x07 );

    const int listNameMaxWidth( 10 );
    const int timeSinceMaxWidth( 4 );
    // Subtract 8 - glyph, space, name, space, (subject), space, time[4 chars].
    const int listSubjectMaxWidth( selectWidth - listNameMaxWidth - 8 );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

Telegram::Telegram( InputDevice& inputDevice,
                    TelegramLoaders::Factory& telegramLoaderFactory,
                    const World::User& worldUser,
                    const Metadata& worldMetadata,
                    const Worldbook& worldbook,
                    AssetLoaders::Factory& assetLoaderFactory,
                    Agape::Editor::Factory& editorFactory,
                    Encryptors::Factory& encryptorFactory,
                    WindowManager& windowManager,
                    const String& windowName,
                    Dialogue& dialogue,
                    Timers::Factory& timerFactory,
                    Clock& clock,
                    Snowflake& snowflake ) :
  m_inputDevice( inputDevice ),
  m_telegramLoaderFactory( telegramLoaderFactory ),
  m_worldUser( worldUser ),
  m_worldMetadata( worldMetadata ),
  m_worldBook( worldbook ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_editorFactory( editorFactory ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_dialogue( dialogue ),
  m_clock( clock ),
  m_snowflake( snowflake ),
  m_encryptor( encryptorFactory.makeEncryptor() ),
  m_state( none ),
  m_prevState( none ),
  m_completed( false ),
  m_timer( timerFactory.makeTimer() ),
  m_terminal( nullptr ),
  m_currentForm( nullptr ),
  m_editor( nullptr ),
  m_telegramLoader( nullptr )
{
}

Telegram::~Telegram()
{
    closeForm();
    delete( m_encryptor );
    delete( m_timer );
    delete( m_editor );
    delete( m_telegramLoader );
}

void Telegram::enter( const Value& parameters )
{
    m_completed = false;

    m_telegramLoader = m_telegramLoaderFactory.makeLoader( m_worldUser.m_snowflake );
    
    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
        if( drawList( false ) ) // false = show inbox.
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

void Telegram::returnTo( const Value& parameters )
{
}

bool Telegram::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool Telegram::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        closeForm();
        m_windowManager.setTerminalWindowVisible( m_windowName, false );
        delete( m_telegramLoader );
        m_telegramLoader = nullptr;
        return true;
    }

    return false;
}

void Telegram::run()
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
            if( ( c == Key::newLine ) && !m_telegrams.empty() )
            {
                if( drawRead() )
                {
                    m_prevState = m_state;
                    m_state = read;
                }
                else
                {
                    drawReadError();
                    m_prevState = m_state;
                    m_state = readError;
                    m_timer->reset();
                }
            }
            else if( c == 'c' )
            {
                drawSelectRecipient();
                m_state = selectRecipient;
            }
            else if( c == 'r' )
            {
                if( prepareReply() )
                {
                    drawEdit();
                    m_state = edit;
                }
            }
            else if( ( c == 'x' ) && !m_telegrams.empty() )
            {
                drawDeletePending();
                if( deleteTelegram() )
                {
                    drawDeleteSuccess();
                    m_timer->reset();
                    m_state = deleteSuccess;
                }
                else
                {
                    drawDeleteError();
                    m_state = deleteError;
                }
            }
            else if( c == 's' )
            {
                drawList( true ); // true = show sent.
                m_state = listSent;
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
        case listSent:
            if( ( c == Key::newLine ) && !m_telegrams.empty() )
            {
                if( drawRead() )
                {
                    m_prevState = m_state;
                    m_state = read;
                }
                else
                {
                    drawReadError();
                    m_prevState = m_state;
                    m_state = readError;
                    m_timer->reset();
                }
            }
            else if( c == Key::escape )
            {
                drawList( false ); // false = show inbox.
                m_state = list;
            }
            else if( m_currentForm )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        case read:
            if( c == Key::escape )
            {
                closeEditor( false );
                drawList( m_prevState == listSent );
                m_state = m_prevState;
            }
            else if( m_editor )
            {
                m_editor->consumeCharacter( c );
            }
            break;
        case selectRecipient:
            if( c == Key::newLine )
            {
                getRecipientSnowflake();
                drawEnterSubject();
                m_state = enterSubject;
            }
            else if( c == Key::escape )
            {
                drawList( false ); // false = show inbox.
                m_state = list;
            }
            else if( m_currentForm )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        case enterSubject:
            if( c == Key::newLine )
            {
                getSubject();
                drawEdit();
                m_state = edit;
            }
            else if( c == Key::escape )
            {
                drawSelectRecipient();
                m_state = selectRecipient;
            }
            else if( m_currentForm )
            {
                m_currentForm->consumeChar( c );
            }
            break;
        case edit:
            if( c == control( 's' ) )
            {
                if( closeEditor( true ) )
                {
                    drawSendPending();
                    if( sendTelegram() )
                    {
                        drawSendSuccess();
                        m_timer->reset();
                        m_state = sendSuccess;
                    }
                    else
                    {
                        drawSendError();
                        m_state = sendError;
                    }
                }
                else
                {
                    drawSaveError();
                    m_state = saveError;
                }
            }
            else if( c == Key::escape )
            {
                drawDiscard();
                m_state = discard;
            }
            else
            {
                if( m_editor )
                {
                    m_editor->consumeCharacter( c );
                }
            }
            break;
        case discard:
            if( c == 'y' )
            {
                closeEditor( false );
                drawList( false ); // false = show inbox.
                m_state = list;
            }
            else
            {
                hideDialogue();
                m_state = edit;
            }
            break;
        case saveError:
            if( c == Key::newLine )
            {
                hideDialogue();
                m_state = edit;
            }
            break;
        case sendError:
            if( c == Key::newLine )
            {
                hideDialogue();
                m_state = edit;
            }
            break;
        case deleteError:
            if( c == Key::newLine )
            {
                hideDialogue();
                drawList( false );  // false = show inbox.
                m_state = list;
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
    else if( m_state == readError )
    {
        if( m_timer->ms() >= 2000 )
        {
            hideDialogue();
            m_state = m_prevState;
        }
    }
    else if( m_state == sendSuccess )
    {
        if( m_timer->ms() >= 2000 )
        {
            hideDialogue();
            drawList( false );  // false = show inbox.
            m_state = list;
        }
    }
    else if( m_state == deleteSuccess )
    {
        if( m_timer->ms() >= 1000 )
        {
            hideDialogue();
            drawList( false ); // false = show inbox.
            m_state = list;
        }
    }
}

void Telegram::drawBackground()
{
    AssetLoaders::Baked assetLoader( World::Coordinates(), backgroundAssetName );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        m_terminal->consumeNext( 0, 0 );
        m_terminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
    }
}

void Telegram::drawLoadPending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Loading" );
    m_dialogue.drawMessage( "Loading telegrams" );
}

bool Telegram::drawList( bool sent )
{
    drawLoadPending();

    m_telegrams.clear();
    bool loaded( false );
    if( !sent )
    {
        loaded = m_telegramLoader->load( m_telegrams );
    }
    else
    {
        loaded = m_telegramLoader->loadSent( m_telegrams );
    }

    if( loaded )
    {
        m_encryptor->setKey( m_worldMetadata.m_itemKey.c_str() );

        LiteStream entryStream;

        std::sort( m_telegrams.begin(), m_telegrams.end(), World::Telegram::newer );
        int newestUnreadIdx( -1 );

        Vector< World::Telegram >::iterator telegramIt( m_telegrams.begin() );
        int currentIdx( 0 );
        for( ; telegramIt != m_telegrams.end(); ++telegramIt , ++currentIdx )
        {
            if( telegramIt != m_telegrams.begin() )
            {
                entryStream << "\x1F"; // Delimiter
            }

            World::Telegram& thisTelegram( *telegramIt );
            thisTelegram.decrypt( *m_encryptor );

            String name;
            int glyph;
            int attributes;
            Vector< User >::const_iterator userIt( m_worldMetadata.m_users.begin() );
            for( ; userIt != m_worldMetadata.m_users.end(); ++userIt )
            {
                if( ( !sent && ( userIt->m_snowflake == thisTelegram.m_senderSnowflake ) ) ||
                    ( sent && ( userIt->m_snowflake == thisTelegram.m_recipientSnowflake ) ) )
                {
                    name = userIt->m_name;
                    glyph = userIt->m_glyph;
                    attributes = userIt->m_attributes;
                    break;
                }
            }

            if( userIt == m_worldMetadata.m_users.end() )
            {
                name = "???";
                glyph = defaultSenderGlyph;
                attributes = defaultSenderAttributes;
            }

            entryStream << ANSITerminal::colour( attributes )
                        << ANSITerminal::charset( Terminal::avatarCharset )
                        << String( 1, (char)glyph )
                        << ANSITerminal::resetCharset()
                        << " ";
            
            String shortName( listNameMaxWidth, ' ' );
            if( name.length() > listNameMaxWidth - 3 )
            {
                shortName = name.substr( 0, listNameMaxWidth - 3 );
                shortName += ".. ";
            }
            else
            {
                shortName.replace( 0, name.length(), name );
            }
            entryStream << shortName << " ";

            if( thisTelegram.m_unread )
            {
                entryStream << ANSITerminal::colour( unreadAttributes );
            }
            else
            {
                entryStream << ANSITerminal::colour( readAttributes );
            }

            String shortSubject( listSubjectMaxWidth, ' ' );
            if( thisTelegram.m_subject.length() > listSubjectMaxWidth - 2 )
            {
                shortSubject = thisTelegram.m_subject.substr( 0, listSubjectMaxWidth - 2 );
                shortSubject += "..";
            }
            else
            {
                shortSubject.replace( 0, thisTelegram.m_subject.length(), thisTelegram.m_subject );
            }
            entryStream << shortSubject;

            String intervalString( Clock::intervalToString( m_clock.epochS() - thisTelegram.m_dateTime ) );
            entryStream << String( timeSinceMaxWidth - intervalString.length(), ' ' );
            entryStream << intervalString;

            if( ( newestUnreadIdx == -1 ) && thisTelegram.m_unread )
            {
                newestUnreadIdx = currentIdx;
            }
        }

        hideDialogue();

        drawBackground();

        Vector< Forms::Field > fields;
        fields.push_back( Forms::Field( _Telegrams,
                                        selectWidth,
                                        selectHeight,
                                        contentFirstRow,
                                        contentCol,
                                        fieldAttributes,
                                        entryStream.str(),
                                        selectionAttributes,
                                        0, // Label attributes
                                        '\x1F' ) ); // Delim

        m_terminal->consumeNext( guideRow, contentCol );
        if( !sent )
        {
            m_terminal->consumeString( "\x1b[0;97;100m\x18/\x19\x1b[37m Sel\x1b[0m  \x1b[97;100mRet\x1b[37m Read\x1b[0m  \x1b[97;100mC\x1b[37m Compose\x1b[0m  \x1b[97;100mR\x1b[37m Reply\x1b[0m", false, Terminal::preserveBackground );
            m_terminal->consumeNext( guideRow + 1, contentCol );
            m_terminal->consumeString( "\x1b[97;100mX\x1b[37m Del\x1b[0m    \x1b[97;100mS\x1b[37m Sent\x1b[0m    \x1b[97;100mEsc\x1b[37m Exit\x1b[0m", false, Terminal::preserveBackground );
        }
        else
        {
            m_terminal->consumeString( "\x1b[0;97;100m\x18/\x19\x1b[37m Select\x1b[0m  \x1b[97;100mRet\x1b[37m Read\x1b[0m  \x1b[97;100mEsc\x1b[37m Back to Inbox\x1b[0m", false, Terminal::preserveBackground );
        }

        m_windowManager.setTerminalWindowVisible( m_windowName, true );

        closeForm();

        m_currentForm = new Forms::Form( m_windowManager,
                                         m_windowName,
                                         String(),
                                         fields,
                                         Forms::Title( sent ? _Telegrams_Sent : _Telegrams_Inbox,
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

            if( newestUnreadIdx != -1 )
            {
                m_currentForm->setSelectIndex( _Telegrams, newestUnreadIdx );
            }
        }

        Map< String, int > numUnread;
        bool allDevices( false );
#ifdef __EMSCRIPTEN__
        allDevices = true;
#endif
        if( m_telegramLoader->unread( numUnread, allDevices ) )
        {
            bool haveUnread( false );

            LiteStream stream;
            stream << "Unread also in ";
            bool needComma( false );
            Map< String, int >::const_iterator it( numUnread.begin() );
            for( ; it != numUnread.end(); ++it )
            {
                if( it->first == m_worldMetadata.m_worldID ) continue;

                String worldName;
                if( m_worldBook.getWorldNameByID( it->first, worldName ) )
                {
                    if( needComma )
                    {
                        stream << ", ";
                    }
                    stream << worldName;
                    needComma = true;
                    haveUnread = true;
                }
            }
            stream << ".";

            if( haveUnread )
            {
                String worldsString( stream.str() );
                String shortWorldsString( worldsString );
                if( worldsString.length() > ( contentMaxWidth - 3 ) )
                {
                    shortWorldsString = worldsString.substr( 0, contentMaxWidth - 3 );
                    shortWorldsString += "...";
                }

                m_terminal->consumeNext( worldsRow, contentCol, 0x0F );
                m_terminal->consumeString( shortWorldsString, Terminal::scrollLock, Terminal::preserveBackground );
            }
        }

        return true;
    }
    else
    {
        hideDialogue();
    }

    return false;
}

void Telegram::drawListError()
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( "Sorry, your telegrams could not be loaded at this time." );
}

bool Telegram::drawRead()
{
    if( m_currentForm )
    {
        int idx( m_currentForm->getSelectIndex( _Telegrams ) );
        if( idx >= 0 )
        {
            const World::Telegram& currentTelegram( m_telegrams[idx] );

            // FIXME: We need to work out how to implement security in the
            // telegram asset loader, so you can't hack the code here and
            // read others' telegrams!
            Coordinates coordinates( m_worldMetadata.m_worldID );
            m_editor = m_editorFactory.makeEditor( coordinates,
                                                   currentTelegram.m_telegramSnowflake,
                                                   String(),
                                                   String(),
                                                   currentTelegram.m_telegramSnowflake,
                                                   "\x1b[0m\x1b[97;100mEsc\x1b[37m Close\x1b[0m" );
            if( m_editor->open( Editor::Editor::modeRead ) )
            {
                closeForm();

                m_editor->draw();

                if( m_state == list ) // not listSent - don't attempt to mark sent telegrams as read!
                {
                    m_telegramLoader->markRead( currentTelegram );
                }

                return true;
            }
        }
    }

    return false;
}

void Telegram::drawReadError()
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( "Sorry, this telegram cannot be opened at this time." );
}

void Telegram::drawSelectRecipient()
{
    LiteStream recipientsStream;
    Vector< User >::const_iterator userIt( m_worldMetadata.m_users.begin() );
    for( ; userIt != m_worldMetadata.m_users.end(); ++userIt )
    {
        if( userIt != m_worldMetadata.m_users.begin() )
        {
            recipientsStream << "\x1F"; // Delimiter
        }

        recipientsStream << ANSITerminal::colour( userIt->m_attributes )
                         << ANSITerminal::charset( Terminal::avatarCharset )
                         << String( 1, (char)userIt->m_glyph )
                         << ANSITerminal::resetCharset()
                         << ANSITerminal::colour( fieldAttributes )
                         << " ";
        
        if( userIt->m_name.length() > selectWidth - 4 )
        {
            recipientsStream << userIt->m_name.substr( 0, selectWidth - 4 );
            recipientsStream << "..";
        }
        else
        {
            recipientsStream << userIt->m_name;
        }
    }
        
    drawBackground();

    Vector< Forms::Field > fields;
    fields.push_back( Forms::Field( _Recipient,
                                    selectWidth,
                                    selectHeight - 1,
                                    contentFirstRow,
                                    contentCol,
                                    fieldAttributes,
                                    recipientsStream.str(),
                                    selectionAttributes,
                                    labelAttributes,
                                    '\x1F' ) ); // Delim

    m_terminal->consumeNext( guideRow, contentCol );
    m_terminal->consumeString( "\x1b[0;97;100m\x18/\x19\x1b[37m Select\x1b[0m  \x1b[97;100mRet\x1b[37m Continue\x1b[0m  \x1b[97;100mEsc\x1b[37m Go back\x1b[0m", false, Terminal::preserveBackground );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                     m_windowName,
                                     String(),
                                     fields,
                                     Forms::Title( _Telegrams,
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

void Telegram::drawEnterSubject()
{
    drawBackground();

    Vector< Forms::Field > fields;
    fields.push_back( Forms::Field( _Subject,
                                    textWidth,
                                    textHeight,
                                    contentFirstRow,
                                    contentCol,
                                    fieldAttributes,
                                    labelAttributes ) );

    m_terminal->consumeNext( guideRow, contentCol );
    m_terminal->consumeString( "\x1b[97;100mRet\x1b[37m Continue\x1b[0m  \x1b[97;100mEsc\x1b[37m Go back\x1b[0m", false, Terminal::preserveBackground );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                     m_windowName,
                                     String(),
                                     fields,
                                     Forms::Title( _Telegrams,
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

bool Telegram::drawEdit()
{
    if( m_currentForm )
    {
        Coordinates coordinates( m_worldMetadata.m_worldID );
        m_editor = m_editorFactory.makeEditor( coordinates,
                                               m_newTelegram.m_telegramSnowflake,
                                               String(),
                                               String(),
                                               m_newTelegram.m_telegramSnowflake,
                                               "\x1b[0m\x1b[97;100mC-S\x1b[37m Save\x1b[0m  \x1b[97;100mEsc\x1b[37m Discard\x1b[0m" );
        if( m_editor->open( Editor::Editor::modeWrite | Editor::Editor::modeCreate ) )
        {
            closeForm();

            m_editor->draw();
            return true;
        }
    }

    return false;
}

void Telegram::drawDiscard()
{
    const char* message( "Are you sure you want to discard this telegram?\
                          Hit \x1b[97mY\x1b[0m to confirm, or\
                          any other key to go back." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Are you sure?" );
    m_dialogue.drawMessage( message );
}

void Telegram::drawSaveError()
{
    const char* message( "Your telegram could not be saved.\
                          Hit \x1b[97mRet\x1b[0m to go back, then\
                          \x1b[97mC-S\x1b[0m to try sending again." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void Telegram::drawSendPending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Sending" );
    m_dialogue.drawMessage( "Sending telegram" );
}

void Telegram::drawSendSuccess()
{
    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( "Telegram sent successfully" );
}

void Telegram::drawSendError()
{
    const char* message( "Your telegram could not be sent.\
                          Hit \x1b[97mRet\x1b[0m to go back, then\
                          \x1b[97mC-S\x1b[0m to try sending again." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void Telegram::drawDeletePending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Deleting" );
    m_dialogue.drawMessage( "Deleting telegram" );
}

void Telegram::drawDeleteSuccess()
{
    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( "Telegram deleted" );
}

void Telegram::drawDeleteError()
{
    const char* message( "This telegram could not be deleted.\
                          Hit \x1b[97mRet\x1b[0m to go back." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void Telegram::hideDialogue()
{
    m_dialogue.hide();
}

bool Telegram::closeEditor( bool save )
{
    if( m_editor->close( save ) )
    {
        delete( m_editor );
        m_editor = nullptr;
        return true;
    }

    return false;
}

void Telegram::closeForm()
{
    if( m_currentForm != nullptr )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

void Telegram::getRecipientSnowflake()
{
    if( m_currentForm )
    {
        int idx( m_currentForm->getSelectIndex( _Recipient ) );
        if( idx >= 0 )
        {
            const User& recipient( m_worldMetadata.m_users[idx] );
            m_newTelegram.m_recipientSnowflake = recipient.m_snowflake;
        }
    }
}

void Telegram::getSubject()
{
    if( m_currentForm )
    {
        m_newTelegram.m_subject = m_currentForm->getFieldContents( _Subject );

        // And set other field values...
        m_newTelegram.m_telegramSnowflake = m_snowflake.generate();
        m_newTelegram.m_senderSnowflake = m_worldUser.m_snowflake;
        m_newTelegram.m_dateTime = m_clock.epochS();
    }
}

bool Telegram::prepareReply()
{
    bool success( false );

    if( m_currentForm )
    {
        int idx( m_currentForm->getSelectIndex( _Telegrams ) );
        if( idx >= 0 )
        {
            const World::Telegram& currentTelegram( m_telegrams[idx] );
            m_newTelegram.m_recipientSnowflake = currentTelegram.m_senderSnowflake;
            m_newTelegram.m_telegramSnowflake = m_snowflake.generate();
            m_newTelegram.m_senderSnowflake = m_worldUser.m_snowflake;
            m_newTelegram.m_dateTime = m_clock.epochS();

            m_newTelegram.m_subject.clear();
            if( currentTelegram.m_subject.find( "Re: " ) == String::npos )
            {
                // Only add Re: if it's not already there.
                m_newTelegram.m_subject = "Re: ";
            }
            m_newTelegram.m_subject += currentTelegram.m_subject;

            success = true;
        }
    }

    return success;
}

bool Telegram::sendTelegram()
{
    // FIXME: Loading assets and scenes, we offload responsibility for encrypt/
    // decrypt to a loader interposed between the user of the loader and the
    // actual loader, to make this transparent and DRY. We should probably do
    // that for TelegramLoaders too.
    m_encryptor->setKey( m_worldMetadata.m_itemKey.c_str() );
    m_newTelegram.encrypt( *m_encryptor );
    return m_telegramLoader->send( m_newTelegram );
}

bool Telegram::deleteTelegram()
{
    bool success( false );

    if( m_currentForm )
    {
        int idx( m_currentForm->getSelectIndex( _Telegrams ) );
        if( idx >= 0 )
        {
            const World::Telegram& currentTelegram( m_telegrams[idx] );

            Coordinates coordinates( m_worldMetadata.m_worldID );
            AssetLoader* assetLoader( m_assetLoaderFactory.makeLoader( coordinates, currentTelegram.m_telegramSnowflake ) );

            success = ( assetLoader->erase() &&
                        m_telegramLoader->erase( currentTelegram ) );

            delete( assetLoader );
        }
    }

    return success;
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
