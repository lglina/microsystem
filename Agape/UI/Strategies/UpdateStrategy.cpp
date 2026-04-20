#include "Actors/NativeActors/NativeActor.h"
#include "AssetLoaders/BakedAssetLoader.h"
#include "Assets/ANSIFile.h"
#include "InputDevices/InputDevice.h"
#include "Loggers/Logger.h"
#include "Memories/Memory.h"
#include "Platforms/Platform.h"
#include "Timers/Factories/TimerFactory.h"
#include "UI/Dialogue.h"
#include "UI/Strategy.h"
#include "Utils/LiteStream.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Tuple.h"
#include "TupleRouter.h"
#include "UpdateStrategy.h"
#include "Value.h"
#include "WindowManager.h"

namespace
{
    const int contentFirstRow( 2 );
    const int contentCol( 1 );
    const int contentMaxWidth( 51 );
    const int contentAttributes( 0x07 );

    const int titleRow( 0 );
    const int titleAttributes( 0x9F );

    const char* backgroundAssetName( "large-dialogue" );

    const int blockSize( 4096 ); // Size of each block to request from server.
} // Anonymous namespace

using namespace Agape::Linda2;
using namespace Agape::World;

namespace Agape
{

namespace UI
{

namespace Strategies
{

Update::Update( InputDevice& inputDevice,
                WindowManager& windowManager,
                const String& windowName,
                TupleRouter& tupleRouter,
                Platform& platform,
                Agape::Memory& updateMemory,
                Dialogue& dialogue,
                Timers::Factory& timerFactory ) :
  Actors::Native( _UpdateClient ),
  m_inputDevice( inputDevice ),
  m_windowManager( windowManager ),
  m_windowName( windowName ),
  m_tupleRouter( tupleRouter ),
  m_platform( platform ),
  m_updateMemory( updateMemory ),
  m_dialogue( dialogue ),
  m_timerFactory( timerFactory ),
  m_state( notification ),
  m_completed( false ),
  m_timer( timerFactory.makeTimer() ),
  m_terminal( nullptr ),
  m_currentVersion( -1 ),
  m_newVersion( -1 ),
  m_size( 0 ),
  m_currentEraseOffset( 0 ),
  m_currentOffset( 0 ),
  m_eraseBytesPerBar( 0 ),
  m_bytesPerBar( 0 ),
  m_currentBar( 0 ),
  m_blockOffset( 0 ),
  m_blockLength( 0 ),
  m_blockEmpty( false )
{
    m_tupleRouter.registerActor( this );
}

Update::~Update()
{
    m_tupleRouter.deregisterActor( this );
    delete( m_timer );
}

void Update::enter( const Value& parameters )
{
    m_completed = false;
    m_state = none;
    m_currentVersion = -1;
    m_newVersion = -1;
    m_size = 0;
    m_currentEraseOffset = 0;
    m_currentOffset = 0;
    m_eraseBytesPerBar = 0;
    m_bytesPerBar = 0;
    m_currentBar = 0;
    m_blockOffset = 0;
    m_blockLength = 0;
    m_blockEmpty = false;

    drawCheck();

    if( needUpdate() )
    {
        hideDialogue();

        WindowManager::TerminalWindow terminalWindow;
        if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
        {
            m_terminal = terminalWindow.m_terminal;

            drawNotification();
            m_state = notification;
        }
        else
        {
            m_completed = true; // Uh oh!
        }
    }
    else
    {
        hideDialogue();

        m_completed = true;
    }
}

void Update::returnTo( const Value& parameters )
{
}

bool Update::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool Update::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        m_windowManager.setTerminalWindowVisible( m_windowName, false );
        return true;
    }

    return false;
}

void Update::run()
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
        case notification:
            if( c == 'y' )
            {
                drawProgress();

                if( openFile() )
                {
                    updateProgress();
                    m_state = erasing;
                }
                else
                {
                    drawError();
                    m_state = error;
                    m_timer->reset();
                }
            }
            else if( c == 'n' )
            {
                m_state = none;
                m_completed = true;
            }
            break;
        default:
            break;
        }
    }

    if( m_state == erasing )
    {
        if( eraseBlock() ) // Erase one block
        {
            updateProgress();
            if( m_currentEraseOffset == m_updateMemory.size() )
            {
                m_state = updating;
            }
            // Otherwise keep reading further sectors next run().
        }
    }
    else if( m_state == updating )
    {
        if( readBlock() ) // Read one block
        {
            updateProgress();
            if( m_currentOffset == m_size ) // If we have all blocks...
            {
                drawSuccess();
                m_state = success;
                m_timer->reset();
            }
            // Otherwise keep reading further blocks next run().
        }
        else
        {
            drawError();
            m_state = error;
            m_timer->reset();
        }
    }
    else if( m_state == success )
    {
        if( m_timer->ms() >= 3000 )
        {
            m_platform.reset();
        }
    }
    else if( m_state == error )
    {
        if( m_timer->ms() >= 3000 )
        {
            hideDialogue();
            m_completed = true;
        }
    }
}

bool Update::accept( Tuple& tuple )
{
    bool handled( false );

    if( TupleRouter::tupleType( tuple ) == _UpdateMetadataResponse )
    {
        LOG_DEBUG( "Update: Received UpdateMetadataResponse" );
        if( m_updateMetadataResponse.set( tuple[_success] ) )
        {
            m_newVersion = tuple[_version];
        }

        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _UpdateOpenResponse )
    {
        LOG_DEBUG( "Update: Received UpdateOpenResponse" );
        if( m_updateOpenResponse.set( tuple[_success] ) )
        {
            m_size = tuple[_size];
        }

        handled = true;
    }
    else if( TupleRouter::tupleType( tuple ) == _UpdateReadResponse )
    {
        LOG_DEBUG( "Update: Received UpdateReadResponse" );
        if( m_updateReadResponse.set( tuple[_success] ) )
        {
            m_blockOffset = tuple[_offset];
            m_blockLength = tuple[_length];
            if( tuple.hasValue( _empty ) )
            {
                m_blockEmpty = true;
            }
            else
            {
                m_blockEmpty = false;
                m_blockData = tuple[_data];
            }
        }

        handled = true;
    }

    return handled;
}

bool Update::perform( Value& returnValue,
                      const String& name,
                      Map< String, Value* > arguments,
                      const String& caller )
{
    return false;
}

bool Update::needUpdate()
{
    bool success( false );

    m_currentVersion = m_platform.buildNumber();

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _UpdateClient );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _UpdateMetadataRequest );

    LOG_DEBUG( "Update: Sending UpdateMetadataRequest" );
    m_updateMetadataResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_updateMetadataResponse.getFuture().get();
    if( success )
    {
        LiteStream stream;
        stream << "Update: Retrieved version. Current version: " << m_currentVersion << " New version: " << m_newVersion;
        LOG_DEBUG( stream.str() );
        return( m_currentVersion < m_newVersion );
    }
    else
    {
        LOG_DEBUG( "Update: Failed to retrieve newest firmware version." );
    }

    return false;
}

bool Update::openFile()
{
    bool success( false );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _UpdateClient );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _UpdateOpenRequest );

    LOG_DEBUG( "Update: Sending UpdateOpenRequest" );
    m_updateOpenResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_updateOpenResponse.getFuture().get();
    if( success )
    {
        LiteStream stream;
        stream << "Update: Opened file. Size: " << m_size;
        LOG_DEBUG( stream.str() );

        m_eraseBytesPerBar = (double)m_updateMemory.size() / ( contentMaxWidth / 2 );
        m_bytesPerBar = (double)m_size / ( contentMaxWidth / 2 );
    }
    else
    {
        LOG_DEBUG( "Update: Failed to open file." );
    }

    return success;
}

bool Update::eraseBlock()
{
    bool success( false );
    success = m_updateMemory.erase( m_currentEraseOffset, m_updateMemory.eraseBlockSize() );
    m_currentEraseOffset += m_updateMemory.eraseBlockSize();

    return success;
}

bool Update::readBlock()
{
    bool success( false );

    Tuple tuple;
    TupleRouter::setSourceActor( tuple, _UpdateClient );
    TupleRouter::setSourceID( tuple, m_tupleRouter.myID() );
    TupleRouter::setTupleType( tuple, _UpdateReadRequest );
    tuple[_offset] = m_currentOffset;
    tuple[_length] = blockSize;

    LOG_DEBUG( "Update: Sending UpdateReadRequest" );
    m_updateReadResponse = Promise( &m_tupleRouter, &m_timerFactory );
    success = m_tupleRouter.route( tuple );
    if( success ) success = m_updateReadResponse.getFuture().get();
    if( success )
    {
        LiteStream stream;
        stream << "Update: Successfully read block. Offset: " << m_currentOffset << " Length: " << m_blockData.rawSize();
        LOG_DEBUG( stream.str() );

        if( !m_blockEmpty )
        {
            success = ( m_blockOffset == m_currentOffset ); // Sanity check.
            if( success ) success = ( m_updateMemory.write( m_currentOffset, m_blockData.raw(), m_blockData.rawSize() ) == m_blockData.rawSize() );
            if( success )
            {
                m_currentOffset += m_blockData.rawSize();
            }
            else
            {
                LiteStream stream;
                stream << "Update: Failed to write block to memory. Offset: " << m_currentOffset << " Length: " << blockSize;
                LOG_DEBUG( stream.str() );
            }
        }
        else
        {
            m_currentOffset += m_blockLength;
        }
    }
    else
    {
        LiteStream stream;
        stream << "Update: Failed to read block. Offset: " << m_currentOffset << " Length: " << blockSize;
        LOG_DEBUG( stream.str() );
    }

    return success;
}

void Update::drawCheck()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Updates" );
    m_dialogue.drawMessage( "Checking for software updates. Please wait." );
}

void Update::hideDialogue()
{
    m_dialogue.hide();
}

void Update::drawBackground()
{
    AssetLoaders::Baked assetLoader( Coordinates(), backgroundAssetName );
    if( assetLoader.open() )
    {
        Assets::ANSIFile ansiFile( assetLoader );
        m_terminal->consumeNext( 0, 0 );
        m_terminal->consumeAsset( ansiFile, ansiFile.dataSize(), true );
    }

    m_terminal->printFormatted( _Update_Available,
                                titleRow,
                                0, // Col.
                                1, // Lines.
                                contentMaxWidth,
                                Terminal::hCentre,
                                Terminal::noVCentre,
                                titleAttributes,
                                Terminal::preserveBackground );
}

void Update::drawNotification()
{
    drawBackground();

    const char* message1( "A software update is available for your computer. \
                           Do you wish to install this update now?" );
    LiteStream message2;
    message2 << "Your current version: " << m_currentVersion;
    LiteStream message3;
    message3 << "Newest version:       " << m_newVersion;
    const char* message4( "Hit \x1b[97mY\x1b[0m to download and install it now, \
                           or hit \x1b[97mN\x1b[0m to decline. 2MiB will be downloaded." );

    m_terminal->printFormatted( message1,
                                contentFirstRow,
                                contentCol,
                                Terminal::noMaxHeight,
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );
    m_terminal->printFormatted( message2.str(),
                                contentFirstRow + 3,
                                contentCol,
                                Terminal::noMaxHeight,
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );
    m_terminal->printFormatted( message3.str(),
                                contentFirstRow + 4,
                                contentCol,
                                Terminal::noMaxHeight,
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );
    m_terminal->printFormatted( message4,
                                contentFirstRow + 6,
                                contentCol,
                                Terminal::noMaxHeight,
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );

    m_windowManager.setTerminalWindowVisible( m_windowName, true );
}

void Update::drawProgress()
{
    drawBackground();

    const char* message1( "Downloading. Please wait." );
    m_terminal->printFormatted( message1,
                                contentFirstRow,
                                contentCol,
                                Terminal::noMaxHeight,
                                contentMaxWidth,
                                Terminal::noHCentre,
                                Terminal::noVCentre,
                                contentAttributes,
                                Terminal::preserveBackground );
    m_terminal->consumeNext( contentFirstRow + 2, contentCol, 0x0F );
    for( int i = 0; i < contentMaxWidth; ++i )
    {
        m_terminal->consumeChar( '\xb0', Terminal::scrollLock, Terminal::preserveBackground );
    }
}

void Update::updateProgress()
{
    if( m_state == erasing )
    {
        int thisBar( m_currentEraseOffset / m_eraseBytesPerBar );
        if( ( thisBar > m_currentBar ) && ( thisBar < ( contentMaxWidth / 2 ) ) )
        {
            m_terminal->consumeNext( contentFirstRow + 2, contentCol + m_currentBar, 0x0F );
            m_terminal->consumeChar( '\xdb', Terminal::scrollLock, Terminal::preserveBackground );
            ++m_currentBar;
        }
    }
    else if( m_state == updating )
    {
        int thisBar( ( contentMaxWidth / 2 ) + ( m_currentOffset / m_bytesPerBar ) );
        if( ( thisBar > m_currentBar ) && ( thisBar < contentMaxWidth ) )
        {
            m_terminal->consumeNext( contentFirstRow + 2, contentCol + m_currentBar, 0x0F );
            m_terminal->consumeChar( '\xdb', Terminal::scrollLock, Terminal::preserveBackground );
            ++m_currentBar;
        }
    }
}

void Update::drawError()
{
    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( "There was an error downloading the update" );
}

void Update::drawSuccess()
{
    m_dialogue.show( Dialogue::success );
    m_dialogue.drawTitle( "Success" );
    m_dialogue.drawMessage( "The update was downloaded successfully. Your computer will restart shortly." );
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
