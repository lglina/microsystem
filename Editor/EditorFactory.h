#ifndef AGAPE_EDITOR_FACTORY_H
#define AGAPE_EDITOR_FACTORY_H

#include "Collections.h"
#include "Editor.h"
#include "String.h"

namespace Agape
{

namespace AssetLoaders
{
class Factory;
} // namespace AssetLoaders

namespace Highlighters
{
class Factory;
} // namespace Highlighters

namespace Timers
{
class Factory;
} // namespace Timers

namespace World
{
class Coordinates;
} // namespace World

class Terminal;

namespace Editor
{

class Factory
{
public:
    Factory( Terminal& terminal,
             AssetLoaders::Factory& assetLoaderFactory,
             Highlighters::Factory* highlighterFactory,
             Timers::Factory& timerFactory,
             Terminal* errorsTerminal = nullptr,
             Terminal* debugLinesTerminal = nullptr,
             Terminal* debugChunksTerminal = nullptr );

    Editor* makeEditor( const World::Coordinates& coordinates,
                        const String& assetName,
                        const String& instanceName,
                        const String& displayName,
                        const String& linkedItem,
                        const String& keyGuide );

private:
    Terminal& m_terminal;
    AssetLoaders::Factory& m_assetLoaderFactory;
    Highlighters::Factory* m_highlighterFactory;
    Timers::Factory& m_timerFactory;
    Terminal* m_errorsTerminal;
    Terminal* m_debugLinesTerminal;
    Terminal* m_debugChunksTerminal;
};

} // namespace Editor

} // namespace Agape

#endif // AGAPE_EDITOR_FACTORY_H
