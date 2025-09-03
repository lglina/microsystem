#include "AssetLoaders/Factories/AssetLoadersFactory.h"
#include "Clocks/Clock.h"
#include "Timers/Factories/TimerFactory.h"
#include "World/User.h"
#include "ANSIEditor.h"
#include "ANSIEditorFactory.h"
#include "Terminal.h"

using namespace Agape::World;

namespace Agape
{

namespace ANSIEditor
{

Factory::Factory( Terminal& terminal,
                  Terminal& toolboxTerminal,
                  Terminal& statusTerminal,
                  AssetLoaders::Factory& assetLoaderFactory,
                  User& user,
                  Timers::Factory& timerFactory,
                  Clock& clock ) :
  m_terminal( terminal ),
  m_toolboxTerminal( toolboxTerminal ),
  m_statusTerminal( statusTerminal ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_user( user ),
  m_timerFactory( timerFactory ),
  m_clock( clock )
{
}

ANSIEditor* Factory::makeEditor( const World::Coordinates& coordinates,
                                 const String& assetName )
{
    return new ANSIEditor( m_terminal,
                           m_toolboxTerminal,
                           m_statusTerminal,
                           m_assetLoaderFactory,
                           coordinates,
                           assetName,
                           m_user.m_name,
                           m_timerFactory,
                           m_clock );
}

} // namespace Editor

} // namespace Agape
