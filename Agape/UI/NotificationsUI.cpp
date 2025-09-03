#include "Platforms/Platform.h"
#include "TelegramLoaders/Factories/TelegramLoadersFactory.h"
#include "TelegramLoaders/TelegramLoader.h"
#include "Utils/LiteStream.h"
#include "World/User.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "NotificationsUI.h"
#include "String.h"
#include "TabBar.h"
#include "Worldbook.h"

namespace Agape
{

using namespace World;

namespace UI
{

NotificationsUI::NotificationsUI( TelegramLoaders::Factory& telegramLoaderFactory,
                                  Coordinates& coordinates,
                                  User& user,
                                  TabBar& tabBar,
                                  Platform& platform,
                                  Worldbook& worldbook ) :
  m_telegramLoaderFactory( telegramLoaderFactory ),
  m_coordinates( coordinates ),
  m_user( user ),
  m_tabBar( tabBar ),
  m_platform( platform ),
  m_worldbook( worldbook ),
  m_previousUnreadTele( 0 ),
  m_userActive( true )
{
}

void NotificationsUI::draw()
{
    poll();
}

void NotificationsUI::poll()
{
    TelegramLoader* telegramLoader( m_telegramLoaderFactory.makeLoader( m_user.m_snowflake ) );
    Map< String, int > numUnread;
    bool allDevices( false );
#ifdef __EMSCRIPTEN__
    allDevices = true;
#endif
    if( telegramLoader->unread( numUnread, allDevices) )
    {
        int totalUnread( 0 );
        Map< String, int >::const_iterator it( numUnread.begin() );
        for( ; it != numUnread.end(); ++it )
        {
            totalUnread += it->second;
        }
 
        if( totalUnread > 0 )
        {
            LiteStream stream;
            stream << totalUnread << " Unread Tele";
            if( m_tabBar.haveTab( _Telegrams ) )
            {
                m_tabBar.update( _Telegrams,
                                 stream.str(),
                                 '\xE0' );
            }
            else
            {
                m_tabBar.create( _Telegrams,
                                 14,
                                 UI::TabBar::right,
                                 true, // true == visible
                                 stream.str(),
                                 '\xE0' );
            }

            if( ( totalUnread > m_previousUnreadTele ) && !m_userActive )
            {
                m_platform.notify( Platform::newTelegram, Platform::client );
            }
        }
        else
        {
            clear();
        }

        m_previousUnreadTele = totalUnread;
    }
    else
    {
        clear();
    }
    delete( telegramLoader );
}

void NotificationsUI::clear()
{
    m_tabBar.remove( _Telegrams );
}

void NotificationsUI::setUserActive( bool active )
{
    if( !m_userActive && active )
    {
        m_platform.cancelNotify( Platform::newTelegram );
    }

    m_userActive = active;
}

} // namespace UI

} // namespace Agape
