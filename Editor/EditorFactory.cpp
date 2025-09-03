#include "Highlighters/Factories/HighlighterFactory.h"
#include "Timers/Factories/TimerFactory.h"
#include "World/Compositor.h"
#include "World/WorldCoordinates.h"
#include "Collections.h"
#include "EditorFactory.h"
#include "Editor.h"
#include "String.h"

namespace Agape
{

namespace Editor
{

Factory::Factory( Terminal& terminal,
                  AssetLoaders::Factory& assetLoaderFactory,
                  Highlighters::Factory* highlighterFactory,
                  Timers::Factory& timerFactory,
                  Terminal* errorsTerminal,
                  Terminal* debugLinesTerminal,
                  Terminal* debugChunksTerminal ) :
  m_terminal( terminal ),
  m_assetLoaderFactory( assetLoaderFactory ),
  m_highlighterFactory( highlighterFactory ),
  m_timerFactory( timerFactory ),
  m_errorsTerminal( errorsTerminal ),
  m_debugLinesTerminal( debugLinesTerminal ),
  m_debugChunksTerminal( debugChunksTerminal )
{
}

Editor* Factory::makeEditor( const World::Coordinates& coordinates,
                             const String& assetName,
                             const String& instanceName,
                             const String& displayName,
                             const String& linkedItem,
                             const String& keyGuide )
{
    return( new Editor( m_terminal,
                        m_assetLoaderFactory,
                        m_highlighterFactory,
                        coordinates,
                        assetName,
                        instanceName,
                        displayName,
                        linkedItem,
                        keyGuide,
                        m_timerFactory,
                        m_errorsTerminal,
                        m_debugLinesTerminal,
                        m_debugChunksTerminal ) );
}

} // namespace Editor

} // namespace Agape
