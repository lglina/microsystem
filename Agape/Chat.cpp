#include "Encryptors/Factories/EncryptorsFactory.h"
#include "Encryptors/Encryptor.h"
#include "Loggers/Logger.h"
#include "Platforms/Platform.h"
#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "UI/Forms/Form.h"
#include "Utils/Cartesian.h"
#include "World/WorldMetadata.h"
#include "World/User.h"
#include "World/WorldCoordinates.h"
#include "Chat.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "TupleRoutingCriteria.h"
#include "TupleRouter.h"
#include "Tuple.h"
#include "WindowManager.h"

using namespace Agape::Linda2;
using namespace Agape::UI;
using namespace Agape::World;

namespace
{
    const long notificationPeriod( 300000 ); // ms.
} // Anonymous namespace

namespace Agape
{

Chat::Chat( Coordinates& coordinates,
            Metadata& worldMetadata,
            User& worldUser,
            Encryptors::Factory& encryptorFactory,
            TupleRouter& tupleRouter,
            WindowManager& windowManager,
            const String& windowName,
            const Point& normalPosition,
            const Point& maximisedPosition,
            Platform& platform,
            Timers::Factory& timerFactory ) :
  Native( _Chat ),
  m_coordinates( coordinates ),
  m_worldMetadata( worldMetadata ),
  m_worldUser( worldUser ),
  m_tupleRouter( tupleRouter ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_normalPosition( normalPosition ),
  m_maximisedPosition( maximisedPosition ),
  m_platform( platform ),
  m_encryptor( encryptorFactory.makeEncryptor() ),
  m_terminal( nullptr ),
  m_notificationLastSent( timerFactory.makeTimer() ),
  m_havePreviousCoordinates( false ),
  m_currentForm( nullptr ),
  m_lineEmpty( true ),
  m_cursorPos( 0 ),
  m_isMaximised( false ),
  m_userActive( true ),
  m_didSendNotification( false )
{
    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }

    m_tupleRouter.registerActor( this );
}

Chat::~Chat()
{
    stop();
    m_tupleRouter.deregisterActor( this );
    delete( m_encryptor );
    delete( m_notificationLastSent );
}

void Chat::coordinatesChanged()
{
    removeRouting();
    addRouting();
    normalSize();
}

void Chat::stop()
{
    removeRouting();
    deleteForm();

    normalSize();

    if( m_terminal )
    {
        m_terminal->clearScreen();
    }
}

void Chat::receiveMessage( const String& senderName,
                           int senderAttributes,
                           const String& message )
{
    if( !m_terminal ) return;

    if( m_currentForm )
    {
        hideForm();
        m_terminal->clearLines( m_terminal->height() - 1, 1 );
    }
    else
    {
        if( m_lineEmpty )
        {
            m_lineEmpty = false;
        }
        else
        {
            m_terminal->scrollScreen( Terminal::scrollDown );
        }
    }

    int effectiveAttributes = ( ( senderAttributes == 0 ) ? 0xF0 : senderAttributes );
    m_terminal->consumeNext( m_terminal->height() - 1, 0, effectiveAttributes );
    m_terminal->consumeString( senderName );
    m_terminal->consumeNext( m_terminal->height() - 1, senderName.size() + 1, 0x07 );
    m_terminal->consumeString( message, Terminal::scrollLock );

    if( m_currentForm )
    {
        m_terminal->scrollScreen( Terminal::scrollDown );
        showForm();
    }

    if( !m_userActive &&
        ( !m_didSendNotification ||
          ( m_notificationLastSent->ms() >= notificationPeriod ) ) )
    {
        m_platform.notify( Platform::newChat, Platform::client );
        m_didSendNotification = true;
        m_notificationLastSent->reset();
    }
}

void Chat::consumeCharacter( char c )
{
    if( c == '\r' )
    {
        // Ignore.
    }
    else if( c == '\n' )
    {
        if( m_currentForm )
        {
            sendMessage();
            deleteForm();
            m_terminal->scrollScreen( Terminal::scrollDown );
            m_lineEmpty = true;
        }
    }
    else if( m_currentForm )
    {
        m_currentForm->consumeChar( c );
    }
    else
    {
        if( !m_lineEmpty )
        {
            m_terminal->scrollScreen( Terminal::scrollDown );
        }
        createForm();
        if( m_currentForm ) m_currentForm->consumeChar( c );
    }
}

void Chat::toggleMaximise()
{
    if( m_isMaximised )
    {
        normalSize();
    }
    else
    {
        maximise();
    }
}

void Chat::maximise()
{
    m_windowManager.moveWindow( m_windowName,
                                m_maximisedPosition );
    m_isMaximised = true;
}

void Chat::normalSize()
{
    m_windowManager.moveWindow( m_windowName,
                                m_normalPosition );
    m_isMaximised = false;
}

void Chat::setUserActive( bool active )
{
    if( !m_userActive && active )
    {
        m_platform.cancelNotify( Platform::newChat );
        m_didSendNotification = false;
    }

    m_userActive = active;
}

bool Chat::accept( Tuple& tuple )
{
    if( ( TupleRouter::tupleType( tuple ) == _ChatMessage ) &&
        ( TupleRouter::sourceID( tuple ) != m_tupleRouter.myID() ) )
    {
        Coordinates coordinates = Coordinates::fromValue( tuple[_coordinates] );
        if( coordinates != m_coordinates )
        {
            LOG_DEBUG( "Received spurious chat message" );
            return false;
        }

        m_encryptor->setKey( m_worldMetadata.m_itemKey.c_str() );

        User user = User::fromValue( tuple[_user] );
        user.decrypt( *m_encryptor );

        if( tuple.hasValue( _message ) )
        {
            String message = tuple[_message];
            message = m_encryptor->decrypt( message );
            receiveMessage( user.m_name, user.m_attributes, message );
            return true;
        }
    }
    else if( ( TupleRouter::tupleType( tuple ) == _ChatMessage ) &&
             ( TupleRouter::sourceActor( tuple ) != _Chat ) )
    {
        // Locally generated.
        String name( "[World]" );
        int attributes( 0x0F );
        if( tuple.hasValue( _name ) && tuple.hasValue( _colour ) )
        {
            name = tuple[_name];
            attributes = Terminal::attributes( tuple[_colour] );
        }

        receiveMessage( name, attributes, tuple[_message] );
        return true;
    }

    return false;
}

void Chat::addRouting()
{
    TupleRoutingCriteria tupleRoutingCriteria;
    tupleRoutingCriteria.m_types.push_back( new Value( _ChatMessage ) );
    m_coordinates.toRoutingCriteria( tupleRoutingCriteria );
    m_tupleRouter.sendAddRoutingCriteriaRequest( tupleRoutingCriteria );

    m_previousCoordinates = m_coordinates;
    m_havePreviousCoordinates = true;
}

void Chat::removeRouting()
{
    if( m_havePreviousCoordinates )
    {
        TupleRoutingCriteria tupleRoutingCriteria;
        tupleRoutingCriteria.m_types.push_back( new Value( _ChatMessage ) );
        m_previousCoordinates.toRoutingCriteria( tupleRoutingCriteria );
        m_tupleRouter.sendRemoveRoutingCriteriaRequest( tupleRoutingCriteria );
        m_havePreviousCoordinates = false;
    }
}

void Chat::createForm()
{
    if( !m_terminal ) return;

    Vector< Forms::Field > fields;
    fields.push_back( Forms::Field( _ChatMessage,
                                    m_terminal->width() - m_worldUser.m_name.size() - 1,
                                    1,
                                    m_terminal->height() - 1,
                                    m_worldUser.m_name.size() + 1,
                                    0x07 ) );

    deleteForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                     m_windowName,
                                     String(),
                                     fields,
                                     Forms::Title() );
    
    m_currentForm->openUI();

    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->draw();
    }

    int effectiveAttributes = ( ( m_worldUser.m_attributes == 0 ) ? 0xF0 : m_worldUser.m_attributes );
    m_terminal->consumeNext( m_terminal->height() - 1, 0, effectiveAttributes );
    m_terminal->consumeString( m_worldUser.m_name );
    m_terminal->consumeNext( m_terminal->height() - 1, m_worldUser.m_name.size() + 1, 0x07 );
}

void Chat::deleteForm()
{
    if( m_currentForm )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

void Chat::hideForm()
{
    if( m_currentForm && m_terminal )
    {
        int row( 0 );
        int col( 0 );
        m_terminal->getCursor( "Terminal", row, col );
        m_cursorPos = col;

        m_currentForm->close();
    }
}

void Chat::showForm()
{
    if( m_currentForm && m_terminal )
    {
        m_currentForm->draw();

        int effectiveAttributes = ( ( m_worldUser.m_attributes == 0 ) ? 0xF0 : m_worldUser.m_attributes );
        m_terminal->consumeNext( m_terminal->height() - 1, 0, effectiveAttributes );
        m_terminal->consumeString( m_worldUser.m_name );
        m_terminal->consumeNext( m_terminal->height() - 1, m_cursorPos, 0x07 );
    }
}

void Chat::sendMessage()
{
    m_encryptor->setKey( m_worldMetadata.m_itemKey.c_str() );

    User user( m_worldUser );
    user.encrypt( *m_encryptor );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _Chat );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _ChatMessage );
    m_coordinates.toValue( tuple[_coordinates] );
    user.toValue( tuple[_user] );
    String message( m_currentForm->getFieldContents( _ChatMessage ) );
    message = m_encryptor->encrypt( message );
    tuple[_message] = message;

    m_tupleRouter.route( tuple );
}

int Chat::makeNameAttributes( int attributes )
{
    // Make sender colour low intensity and set as background
    // colour for name, with foreground text bright white.
    int nameAttributes( 0 );
    if( attributes & 0x08 )
    {
        attributes &= 0x07;
    }
    nameAttributes = ( attributes << 4 ) + 0x0F;
    return nameAttributes;
}

} // namespace Agape
