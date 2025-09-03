#ifndef AGAPE_EDITOR_HIGHLIGHTERS_FACTORIES_CARLO_H
#define AGAPE_EDITOR_HIGHLIGHTERS_FACTORIES_CARLO_H

#include "HighlighterFactory.h"

namespace Agape
{

namespace Carlo
{
class FunctionDispatcher;
class ProgramManager;
} // namespace Carlo

namespace Linda2
{
class TupleRouter;
} // namespace Linda2

using namespace Carlo;
using namespace Linda2;

namespace Editor
{

namespace Highlighters
{

namespace Factories
{

class Carlo : public Factory
{
public:
    Carlo( TupleRouter& tupleRouter,
           FunctionDispatcher& functionDispatcher,
           Agape::Carlo::ProgramManager& programManager );

    virtual Highlighter* makeHighlighter();

private:
    TupleRouter& m_tupleRouter;
    FunctionDispatcher& m_functionDispatcher;
    Agape::Carlo::ProgramManager& m_programManager;
};

} // namespace Factories

} // namespace Highlighters

} // namespace Editor

} // namespace Agape

#endif // AGAPE_EDITOR_HIGHLIGHTERS_FACTORIES_CARLO_H
