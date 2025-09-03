#include "Clocks/Clock.h"
#include "PresenceLoaders/Factories/PresenceLoadersFactory.h"
#include "PresenceLoaders/PresenceLoader.h"
#include "Utils/LiteStream.h"
#include "World/ScenePresence.h"
#include "World/WorldCoordinates.h"
#include "World/WorldMetadata.h"
#include "Collections.h"
#include "PresenceUI.h"
#include "String.h"
#include "Terminal.h"
#include "WindowManager.h"

#include <algorithm>

namespace
{
    const int defaultAttributes( 0x07 );
    const int showNowSecs( 60 );
} // Anonymous namespace

namespace Agape
{

using namespace World;

namespace UI
{

Presence::Presence( WindowManager& windowManager,
                    const String& windowName,
                    Coordinates& coordinates,
                    Metadata& metadata,
                    PresenceLoaders::Factory& presenceLoaderFactory,
                    Clock& clock ) :
  m_coordinates( coordinates ),
  m_metadata( metadata ),
  m_presenceLoaderFactory( presenceLoaderFactory ),
  m_clock( clock ),
  m_terminal( nullptr ),
  m_presenceLoader( nullptr )
{
    WindowManager::TerminalWindow terminalWindow;
    if( windowManager.getTerminalWindow( windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }
}

Presence::~Presence()
{
    clear();
}

void Presence::draw()
{
    if( !m_terminal ) return;

    if( m_currentCoordinates != m_coordinates )
    {
        delete( m_presenceLoader );
        m_presenceLoader = nullptr;
        m_currentCoordinates = m_coordinates;
    }

    if( !m_presenceLoader )
    {
        m_presenceLoader = m_presenceLoaderFactory.makeLoader( m_coordinates, PresenceLoader::noReceiveRequests );
    }

    Vector< ScenePresence > scenePresences;
    if( !m_presenceLoader->loadWorld( scenePresences ) ) return; // Uh oh!

    m_terminal->clearScreen();

    std::sort( scenePresences.begin(), scenePresences.end(), ScenePresence::newer );

    Vector< ScenePresence >::const_iterator it( scenePresences.begin() );
    int row( 0 );
    for( ; ( it != scenePresences.end() ) && ( row <= ( m_terminal->height() - 3 ) ); ++it, row += 3 )
    {
        ScenePresence presence( *it );

        int attributes( defaultAttributes );
        if( presence.m_present )
        {
            attributes = ( ( presence.m_user.m_attributes == 0 ) ? 0xF0 : presence.m_user.m_attributes );
        }

        // Line 1 - glyph and name.
        m_terminal->consumeNext( row, 0, attributes );
        m_terminal->consumeChar( presence.m_user.m_glyph,
                                 Terminal::scrollUnlock,
                                 Terminal::noCharmode,
                                 nullptr,
                                 0,
                                 Terminal::avatarCharset );
        m_terminal->consumeChar( ' ' );

        if( presence.m_user.m_name.length() <= ( m_terminal->width() - 2 ) )
        {
            m_terminal->consumeString( presence.m_user.m_name );
        }
        else
        {
            m_terminal->consumeString( presence.m_user.m_name.substr( 0, ( m_terminal->width() - 2 ) ) );
        }

        // Line 2 - Position.
        {
        LiteStream stream;
        stream << std::abs( presence.m_coordinates.m_y );
        if( presence.m_coordinates.m_y >= 0 )
        {
            stream << "N ";
        }
        else
        {
            stream << "S ";
        }

        stream << std::abs( presence.m_coordinates.m_x );
        if( presence.m_coordinates.m_x >= 0 )
        {
            stream << "E";
        }
        else
        {
            stream << "W";
        }

        String coordsStr( stream.str() );
        if( coordsStr.length() > m_terminal->width() )
        {
            coordsStr = coordsStr.substr( 0, m_terminal->width() - 3 ) + "...";
        }

        m_terminal->consumeNext( row + 1, 0, defaultAttributes );
        m_terminal->consumeString( coordsStr );
        }

        // Line 3 - Time last seen.
        int sinceLastSeen( m_clock.epochS() - presence.m_lastSeen );
        m_terminal->consumeNext( row + 2, 0, defaultAttributes );
        if( sinceLastSeen >= 0 ) // Might be negative on web if tab was inactive and we elided clock ticks!
        {
            if( presence.m_present && ( sinceLastSeen <= showNowSecs ) )
            {
                m_terminal->consumeString( "now" );
            }
            else if( sinceLastSeen <= ( 60 * 60 * 24 * 7 * 52 ) )
            {
                m_terminal->consumeString( Clock::intervalToString( sinceLastSeen ) );
            }
            else
            {
                m_terminal->consumeString( "never" );
            }
        }
    }

    // Load other users from world metadata and, for those not
    // already shown above, show them now as "never".
    {
    Vector< World::User >::const_iterator userIt( m_metadata.m_users.begin() );
    for( ; ( userIt != m_metadata.m_users.end() && ( row <= ( m_terminal->height() - 2 ) ) ); ++userIt )
    {
        Vector< ScenePresence >::const_iterator presenceIt( scenePresences.begin() );
        for( ; presenceIt != scenePresences.end(); ++presenceIt )
        {
            if( presenceIt->m_user == *userIt )
            {
                break;
            }
        }

        if( presenceIt == scenePresences.end() )
        {
            // Line 1 - glyph and name.
            m_terminal->consumeNext( row, 0, defaultAttributes );
            m_terminal->consumeChar( userIt->m_glyph,
                                     Terminal::scrollUnlock,
                                     Terminal::noCharmode,
                                     nullptr,
                                     0,
                                     Terminal::avatarCharset );
            m_terminal->consumeChar( ' ' );

            if( userIt->m_name.length() <= ( m_terminal->width() - 2 ) )
            {
                m_terminal->consumeString( userIt->m_name );
            }
            else
            {
                m_terminal->consumeString( userIt->m_name.substr( 0, 8 ) );
            }

            // Line 2 - never.
            m_terminal->consumeNext( row + 1, 0, defaultAttributes );
            m_terminal->consumeString( "never" );

            row += 2;
        }
    }
    }
}

void Presence::clear()
{
    delete( m_presenceLoader );
    m_presenceLoader = nullptr;
    
    if( m_terminal )
    {
        m_terminal->clearScreen();
    }
}

} // namespace UI

} // namespace Agape
