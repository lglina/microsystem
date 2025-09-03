#include "AssetLoaders/AssetLoader.h"
#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "Assets/Asset.h"
#include "InputDevices/InputDevice.h"
#include "Loggers/Logger.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "UI/Forms/Form.h"
#include "UI/Dialogue.h"
#include "Utils/base64/base64.h"
#include "World/WorldMetadata.h"
#include "InviteFriendStrategy.h"
#include "KeyUtilities.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "TupleRouter.h"
#include "Tuple.h"
#include "Value.h"
#include "WindowManager.h"
#include "Worldbook.h"

namespace
{
    const int contentFirstRow( 2 );
    const int contentCol( 1 );
    const int contentMaxWidth( 51 );
    const int contentAttributes( 0x07 );
    
    const int textWidth( 50 );
    const int textHeight( 1 );
    const int fieldAttributes( 0x07 );

    const int titleRow( 0 );
    const int titleAttributes( 0x9F );

    const int guideRow( 16 );

    const char* backgroundAssetName( "large-dialogue" );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

InviteFriend::InviteFriend( InputDevice& inputDevice,
              Worldbook& worldbook,
              const Metadata& worldMetadata,
              KeyUtilities& keyUtilities,
              TupleRouter& tupleRouter,
              WindowManager& windowManager,
              const String& windowName,
              Dialogue& dialogue,
              Timers::Factory& timerFactory ) :
  Actors::Native( _InviteFriendClient ),
  m_inputDevice( inputDevice ),
  m_worldbook( worldbook ),
  m_worldMetadata( worldMetadata ),
  m_keyUtilities( keyUtilities ),
  m_tupleRouter( tupleRouter ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_dialogue( dialogue ),
  m_timerFactory( timerFactory ),
  m_terminal( nullptr ),
  m_timer( timerFactory.makeTimer() ),
  m_currentForm( nullptr ),
  m_state( none ),
  m_completed( false )
{
    m_tupleRouter.registerActor( this );
}

InviteFriend::~InviteFriend()
{
    m_tupleRouter.deregisterActor( this );
    closeForm();
    delete( m_timer );
}

void InviteFriend::enter( const Value& parameters )
{
    m_completed = false;

    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;

        drawEnterDetails();
        m_state = enterDetails;
    }
    else
    {
        m_completed = true; // Uh oh!
    }
}

void InviteFriend::returnTo( const Value& parameters )
{
}

bool InviteFriend::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool InviteFriend::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        closeForm();
        m_windowManager.setTerminalWindowVisible( m_windowName, false );
        return true;
    }

    return false;
}

void InviteFriend::run()
{
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }

        switch( m_state )
        {
        case enterDetails:
            if( c == '\n' )
            {
                if( m_currentForm )
                {
                    if( formComplete() )
                    {
                        drawPending();
                        if( doInvite() )
                        {
                            drawSuccess();
                            m_timer->reset();
                            m_state = success;
                        }
                        else
                        {
                            drawError();
                            m_timer->reset();
                            m_state = error;
                        }
                    }
                    else
                    {
                        m_currentForm->nextField();
                    }
                }
            }
            else if( c == '\x1b' )
            {
                m_completed = true;
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

    if( m_state == success )
    {
        if( m_timer->ms() >= 5000 )
        {
            hideDialogue();
            m_completed = true;
        }
    }
    else if( m_state == error )
    {
        if( m_timer->ms() >= 3000 )
        {
            hideDialogue();
            m_state = enterDetails;
        }
    }
}

void InviteFriend::str( LiteStream& stream, int indent )
{
}

bool InviteFriend::accept( Tuple& tuple )
{
    bool handled( false );

    if( TupleRouter::tupleType( tuple ) == _InviteFriendResponse )
    {
        LOG_DEBUG( "InviteFriend: Received invite friend response" );
        m_inviteFriendResponse.set( tuple[_success] );

        handled = true;
    }

    return handled;
}

bool InviteFriend::perform( Value& returnValue,
                      const String& name,
                      Map< String, Value* > arguments,
                      const String& caller )
{
    return false;
}

String InviteFriend::actorName() const
{
    return _InviteFriendClient;
}

void InviteFriend::drawBackground()
{
    AssetLoaders::Baked assetLoader( Coordinates(), backgroundAssetName );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        m_terminal->consumeNext( 0, 0 );
        m_terminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
    }

    m_terminal->printFormatted( _Invite_A_Friend,
                                titleRow,
                                0, // Col.
                                1, // Lines.
                                contentMaxWidth,
                                Terminal::hCentre,
                                Terminal::noVCentre,
                                titleAttributes,
                                Terminal::preserveBackground );
}

void InviteFriend::drawEnterDetails()
{
    drawBackground();

    m_terminal->consumeNext( guideRow, contentCol );
    m_terminal->consumeString( "\x1b[0;97;100mTab/Ret\x1b[37m Next\x1b[0m  \x1b[97;100mEsc\x1b[37m Go back\x1b[0m", false, Terminal::preserveBackground );

    const char* message1( "So you want to invite a friend? Great! We'll get some details\
                          from you now, then we'll send your friend an invitation by email.\
                          Hit \x1b[97mRet\x1b[0m or \x1b[97mTab\x1b[0m after you answer each\
                          question, or hit \x1b[97mEsc\x1b[0m to go back." );
    const char* message2( "What's your name?" );
    const char* message3( "What's your friend's name?" );
    const char* message4( "What's your friend's email address?" );

    m_terminal->printFormatted( message1,
                                contentFirstRow,
                                contentCol,
                                Terminal::noMaxHeight,
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );
    m_terminal->printFormatted( message2,
                                contentFirstRow + 5,
                                contentCol,
                                Terminal::noMaxHeight,
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );
    m_terminal->printFormatted( message3,
                                contentFirstRow + 8,
                                contentCol,
                                Terminal::noMaxHeight,
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );
    m_terminal->printFormatted( message4,
                                contentFirstRow + 11,
                                contentCol,
                                Terminal::noMaxHeight,
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );
    
    Vector< Forms::Field > fields;
    fields.push_back( Forms::Field( _name,
                                    textWidth,
                                    textHeight,
                                    contentFirstRow + 6,
                                    contentCol,
                                    fieldAttributes ) );
    fields.push_back( Forms::Field( _friendsName,
                                    textWidth,
                                    textHeight,
                                    contentFirstRow + 9,
                                    contentCol,
                                    fieldAttributes ) );
    fields.push_back( Forms::Field( _friendsEmail,
                                    textWidth,
                                    textHeight,
                                    contentFirstRow + 12,
                                    contentCol,
                                    fieldAttributes ) );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                     m_windowName,
                                     String(),
                                     fields,
                                     Forms::Title( _Invite_A_Friend,
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

    m_windowManager.setTerminalWindowVisible( m_windowName, true );
}

void InviteFriend::drawPending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Sending" );
    m_dialogue.drawMessage( "Sending invitation. Please wait." );
}

void InviteFriend::drawSuccess()
{
    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( "The invitation was successfully sent. Please ask your friend to check their email. Hit C-F to invite another." );
}

void InviteFriend::drawError()
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( "We're sorry, but we were unable to send an invitation at this time. Please try again later." );
}

bool InviteFriend::formComplete()
{
    String name( m_currentForm->getFieldContents( _name ) );
    String friendsName( m_currentForm->getFieldContents( _friendsName ) );
    String friendsEmail( m_currentForm->getFieldContents( _friendsEmail ) );

    return( !name.empty() && !friendsName.empty() && !friendsEmail.empty() );
}

void InviteFriend::hideDialogue()
{
    m_dialogue.hide();
}

void InviteFriend::closeForm()
{
    if( m_currentForm != nullptr )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

bool InviteFriend::doInvite()
{
    bool success( true );

    String name( m_currentForm->getFieldContents( _name ) );
    String friendsName( m_currentForm->getFieldContents( _friendsName ) );
    String friendsEmail( m_currentForm->getFieldContents( _friendsEmail ) );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _InviteFriendClient );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _InviteFriendRequest );
    tuple[_name] = name;
    tuple[_friendsName] = friendsName;
    tuple[_friendsEmail] = friendsEmail;

    // Generate random "sealing" key to encrypt world key.
    char sealingKey[m_keyUtilities.sealingKeySize()];
    m_keyUtilities.generateSealingKey( sealingKey );

    // Get world key for current world.
    World::Metadata metadata;
    char sealedWorldKey[m_keyUtilities.sealedWorldKeySize()];
    if( m_worldbook.getMetadata( m_worldMetadata.m_worldID, metadata ) )
    {
        // Encrypt world key with random "sealing" key.
        m_keyUtilities.sealWorldKey( &metadata.m_worldKey[0], sealingKey, sealedWorldKey );

        // Base-64 encode sealing key and sealed world key.
        String sealingKeyEncoded( Base64encode_len( m_keyUtilities.sealingKeySize() ), '\0' );
        Base64encode( &sealingKeyEncoded[0], sealingKey, m_keyUtilities.sealingKeySize() );
        sealingKeyEncoded.resize( sealingKeyEncoded.length() - 1 );

        String sealedWorldKeyEncoded( Base64encode_len( m_keyUtilities.sealedWorldKeySize() ), '\0' );
        Base64encode( &sealedWorldKeyEncoded[0], sealedWorldKey, m_keyUtilities.sealedWorldKeySize() );
        sealedWorldKeyEncoded.resize( sealedWorldKeyEncoded.length() - 1 );

        // Send to server. Sealed world key will be saved at the server and the
        // sealing key will be sent via email. The server then can't decrypt
        // the world key (except for the short time it has both parts), and the
        // world key is only decrypted within the invitee's web browser.
        tuple[_sealingKey] = sealingKeyEncoded;
        tuple[_sealedWorldKey] = sealedWorldKeyEncoded;
    }
    else
    {
        success = false;
    }

    if( success )
    {
        m_inviteFriendResponse = Promise( &m_tupleRouter, &m_timerFactory );
        success = m_tupleRouter.route( tuple );
        if( success ) success = m_inviteFriendResponse.getFuture().get();
    }

    return success;
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
