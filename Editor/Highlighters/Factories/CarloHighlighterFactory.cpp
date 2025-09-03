#include "Highlighters/CarloHighlighter.h"
#include "CarloHighlighterFactory.h"
#include "FunctionDispatcher.h"
#include "ProgramManager.h"
#include "TupleRouter.h"

namespace Agape
{

namespace Editor
{

namespace Highlighters
{

namespace Factories
{

Carlo::Carlo( TupleRouter& tupleRouter,
              FunctionDispatcher& functionDispatcher,
              Agape::Carlo::ProgramManager& programManager ) :
  m_tupleRouter( tupleRouter ),
  m_functionDispatcher( functionDispatcher ),
  m_programManager( programManager )
{
}

Highlighter* Carlo::makeHighlighter()
{
    return new Highlighters::Carlo( m_tupleRouter, m_functionDispatcher, m_programManager );
}

} // namespace Factories

} // namespace Highlighters

} // namespace Editor

} // namespace Agape
