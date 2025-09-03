#ifndef AGAPE_CONNECTION_MONITOR_H
#define AGAPE_CONNECTION_MONITOR_H

#include "Lines/Line.h"
#include "Runnable.h"

namespace Agape
{

namespace Linda2
{
class TupleRoute;
class TupleRouter;
} // namespace Linda2

namespace UI
{
class TabBar;
class VRTime;
} // namespace UI

class ConfigurationStore;

using namespace Linda2;

class ConnectionMonitor : public Runnable
{
public:
    ConnectionMonitor( Line& line,
                       ConfigurationStore& configurationStore,
                       TupleRouter& tupleRouter,
                       TupleRoute& tupleRoute,
                       UI::TabBar& tabBar,
                       UI::VRTime& vrTime );
    virtual ~ConnectionMonitor() {}

    virtual void run();

private:
    void onConnect();
    void onDisconnect();

    void updateStatusBar();

    Line& m_line;
    ConfigurationStore& m_configurationStore;
    TupleRouter& m_tupleRouter;
    TupleRoute& m_tupleRoute;
    UI::TabBar& m_tabBar;
    UI::VRTime& m_vrTime;

    struct Line::LineStatus m_lineStatus;
};

} // namespace Agape

#endif // AGAPE_CONNECTION_MONITOR_H
