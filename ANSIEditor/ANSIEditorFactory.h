#ifndef AGAPE_ANSI_EDITOR_FACTORY_H
#define AGAPE_ANSI_EDITOR_FACTORY_H

#include "ANSIEditor.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{
class Factory;
} // namespace AssetLoaders

namespace Timers
{
class Factory;
} // namespace Timers

namespace World
{
class Coordinates;
class User;
} // namespace World

class Clock;
class Terminal;

using namespace World;

namespace ANSIEditor
{

class Factory
{
public:
    Factory( Terminal& terminal,
             Terminal& toolboxTerminal,
             Terminal& statusTerminal,
             AssetLoaders::Factory& assetLoaderFactory,
             User& user,
             Timers::Factory& timerFactory,
             Clock& clock );

    ANSIEditor* makeEditor( const World::Coordinates& coordinates,
                            const String& assetName );

private:
    Terminal& m_terminal;
    Terminal& m_toolboxTerminal;
    Terminal& m_statusTerminal;
    AssetLoaders::Factory& m_assetLoaderFactory;
    User& m_user;
    Timers::Factory& m_timerFactory;
    Clock& m_clock;
};

} // namespace Editor

} // namespace Agape

#endif // AGAPE_ANSI_EDITOR_FACTORY_H
