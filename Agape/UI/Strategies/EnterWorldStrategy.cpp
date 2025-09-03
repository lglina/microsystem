#include "InputDevices/InputDevice.h"
#include "Lines/Line.h"
#include "Loggers/Logger.h"
#include "UI/Dialogue.h"
#include "World/User.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "World/WorldUtilities.h"
#include "WorldLoaders/Factories/WorldLoadersFactory.h"
#include "EnterWorldStrategy.h"
#include "ConfigurationStore.h"
#include "Phonebook.h"
#include "String.h"
#include "StringConstants.h"
#include "Value.h"
#include "Worldbook.h"

#include <string.h>

namespace Agape
{

namespace UI
{

namespace Strategies
{

EnterWorld::EnterWorld( InputDevice& inputDevice,
                        Worldbook& worldbook,
                        Line& line,
                        Metadata& sessionMetadata,
                        User& sessionUser,
                        World::Utilities& worldUtilities,
                        WorldLoaders::Factory& worldLoaderFactory,
                        ConfigurationStore& configurationStore,
                        Dialogue& dialogue ) :
  m_inputDevice( inputDevice ),
  m_worldbook( worldbook ),
  m_line( line ),
  m_sessionMetadata( sessionMetadata ),
  m_sessionUser( sessionUser ),
  m_worldUtilities( worldUtilities ),
  m_worldLoaderFactory( worldLoaderFactory ),
  m_configurationStore( configurationStore ),
  m_dialogue( dialogue ),
  m_state( none ),
  m_completed( false ),
  m_calling( false )
{
}

void EnterWorld::enter( const Value& parameters )
{
    m_completed = false;
    m_parameters = parameters;
    m_callingParameters = Value();

    m_worldID.clear();
    if( m_parameters[_worldID] == String( _default ) )
    {
        LOG_DEBUG( "EnterWorld: enter: Entering default world" );
        if( !getHomeLocation() &&
            !getDefaultWorld() )
        {
            // Default world explicitly requested. Show failure message.
            m_state = error;
            drawErrorNoDefault();
        }
    }
    else if( m_parameters[_worldID].toString().empty() )
    {
        LOG_DEBUG( "EnterWorld: enter: Entering default world" );
        if( !getHomeLocation() &&
            !getDefaultWorld() )
        {
            // Menu was just probing to see if there is a default.
            // Silently return to menu.
            m_completed = true;
        }
    }
    else
    {
        m_worldID = m_parameters[_worldID];
        LOG_DEBUG( "EnterWorld: enter: Entering specified world " + m_worldID );
    }

    if( !m_worldID.empty() )
    {
        Line::LineStatus lineStatus( m_line.getLineStatus() );
        if( lineStatus.m_carrier )
        {
            m_state = pending;
            drawPending();
        }
        else
        {
            m_state = connect;
            m_nextStrategy = "connect";
            m_calling = true;
        }
    }
}

void EnterWorld::returnTo( const Value& parameters )
{
    m_calling = false;
    m_nextStrategy.clear();

    if( m_state == connect )
    {
        if( (int)parameters[_success] == 1 )
        {
            LOG_DEBUG( "EnterWorld: returnTo: Connect succeeded." );
            m_state = pending;
            drawPending();
        }
        else
        {
            LOG_DEBUG( "EnterWorld: returnTo: Connect failed. Returning from EnterWorld." );
            m_completed = true;
        }
    }
    else if( m_state == walk )
    {
        if( (int)parameters[_doTeleport] == 1 )
        {
            LOG_DEBUG( "EnterWorld: returnTo: Teleport requested. Setting worldID and calling WalkStrategy." );
            LOG_DEBUG( parameters.dump() );
            // We teleported to a different world. Re-enter to get to the
            // new world.
            m_callingParameters = parameters; // For WalkStrategy to get coordinates and row,col.
            m_worldID = World::Coordinates::fromValue( parameters[_coordinates] ).m_worldID;
            m_state = pending;
            drawPending();
        }
        else
        {
            LOG_DEBUG( "EnterWorld: returnTo: Teleport not requested. Returning from EnterWorld." );
            m_completed = true;
        }
    }
}

bool EnterWorld::calling( String& strategyName, Value& parameters )
{
    if( m_calling )
    {
        strategyName = m_nextStrategy;
        parameters = m_callingParameters;
        
        return true;
    }

    return false;
}

bool EnterWorld::returning( String& nextStrategy, Value& parameters )
{
    return( m_completed );
}

void EnterWorld::run()
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
        case error:
            if( c == '\n' )
            {
                m_completed = true;
                hideDialogue();
            }
            break;
        default:
            break;
        }
    }

    if( m_state == pending )
    {
        if( enterWorld( m_worldID, m_sessionUser, m_sessionMetadata ) )
        {
            hideDialogue();
            m_state = walk;
            m_nextStrategy = "walk";
            m_calling = true;
        }
        else
        {
            m_state = error;
            drawErrorEnter();
        }
    }
}

bool EnterWorld::getHomeLocation()
{
    if( m_configurationStore.hasKey( _home ) )
    {
        Value& homeValue( m_configurationStore.get( _home ) );
        if( homeValue.hasValue( _coordinates ) &&
            homeValue.hasValue( _row ) &&
            homeValue.hasValue( _column ) )
        {
            m_callingParameters[_coordinates] = homeValue[_coordinates];
            m_callingParameters[_row] = homeValue[_row];
            m_callingParameters[_column] = homeValue[_column];
            m_worldID = homeValue[_coordinates][_worldID];
            return true;
        }
    }

    return false;
}

bool EnterWorld::getDefaultWorld()
{
    if( m_worldbook.hasDefaultWorldID() )
    {
        m_worldID = m_worldbook.getDefaultWorldID();
        return true;
    }

    return false;
}

bool EnterWorld::enterWorld( const String& worldID, User& sessionUser, Metadata& sessionMetadata )
{
    bool worldLoaded( false );
    Metadata worldBookMetadata;
    if( m_worldbook.getMetadata( worldID, worldBookMetadata ) &&
        m_worldbook.getUserForWorld( worldID, sessionUser ) )
    {
        if( worldBookMetadata.m_worldKey.length() == 16 ) // Ensure we have a key.
        {
            char worldKey[16];
            ::memcpy( worldKey, &worldBookMetadata.m_worldKey[0], 16 );

            World::Metadata encryptedMetadata;
            m_worldUtilities.encryptMetadata( worldKey, worldBookMetadata, encryptedMetadata );

            LOG_DEBUG( "Loading world" );
            WorldLoader* worldLoader( m_worldLoaderFactory.makeLoader() );
            String reason;
            if( worldLoader->load( encryptedMetadata, reason ) )
            {
                LOG_DEBUG( "Loaded successfully" );
                m_worldUtilities.decryptMetadata( worldKey, encryptedMetadata, m_sessionMetadata );
                worldLoaded = true;
            }
            else
            {
                LOG_DEBUG( "Failed to load world" );
            }
            delete( worldLoader );
        }
    }

    return worldLoaded;
}

void EnterWorld::drawPending()
{
    m_dialogue.show( Dialogue::normal );
    m_dialogue.drawTitle( "Please Wait" );
    m_dialogue.drawMessage( "Entering world" );
}

void EnterWorld::drawErrorNoDefault()
{
    const char* message( "No world is configured. Create or join a world first.\
                          \x1b[97mRet\x1b[0m to go back." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void EnterWorld::drawErrorEnter()
{
    const char* message( "Unable to enter world. Please check your internet connection.\
                          If the problem persists, contact support.\
                          \x1b[97mRet\x1b[0m to go back." );

    m_dialogue.show( Dialogue::error );
    m_dialogue.drawTitle( "Error" );
    m_dialogue.drawMessage( message );
}

void EnterWorld::hideDialogue()
{
    m_dialogue.hide();
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
