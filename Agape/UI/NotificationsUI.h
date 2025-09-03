#ifndef AGAPE_UI_NOTIFICATIONS_H
#define AGAPE_UI_NOTIFICATIONS_H

namespace Agape
{

namespace TelegramLoaders
{
class Factory;
} // namespace TelegramLoaders

namespace World
{
class Coordinates;
class User;
} // namespace World

class Platform;
class Worldbook;

using namespace World;

namespace UI
{

class TabBar;

class NotificationsUI
{
public:
    NotificationsUI( TelegramLoaders::Factory& telegramLoaderFactory,
                     Coordinates& coordinates,
                     User& user,
                     TabBar& tabBar,
                     Platform& platform,
                     Worldbook& worldBook );

    void draw();
    void poll();
    void clear();

    void setUserActive( bool active );

private:
    TelegramLoaders::Factory& m_telegramLoaderFactory;
    Coordinates& m_coordinates;
    User& m_user;
    TabBar& m_tabBar;
    Platform& m_platform;
    Worldbook& m_worldbook;

    int m_previousUnreadTele;
    bool m_userActive;
};

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_NOTIFICATIONS_H
