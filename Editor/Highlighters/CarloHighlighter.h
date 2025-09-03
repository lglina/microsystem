#ifndef AGAPE_EDITOR_HIGHLIGHTERs_CARLO_H
#define AGAPE_EDITOR_HIGHLIGHTERS_CARLO_H

#include "Highlighters/Highlighter.h"
#include "Collections.h"
#include "Lexer.h"
#include "String.h"

using namespace Agape::Carlo;

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

using namespace Linda2;

namespace Editor
{

namespace Highlighters
{

class Carlo : public Highlighter
{
public:
    Carlo( TupleRouter& tupleRouter,
           FunctionDispatcher& functionDispatcher,
           Agape::Carlo::ProgramManager& programManager );

    virtual void line( const String& line, int logicalLine );
    virtual void highlight( const String& instanceName,
                            bool modified,
                            Vector< Token >& highlights,
                            Vector< String >& parseErrors,
                            Vector< String >& runtimeErrors );

private:
    TupleRouter& m_tupleRouter;
    FunctionDispatcher& m_functionDispatcher;
    Agape::Carlo::ProgramManager& m_programManager;

    Lexer m_lexer;
    Deque< Lexer::Token > m_lexerTokens;

    int m_previousLogicalLine;
};

} // namespace Highlighters

} // namespace Editor

} // namespace Agape

#endif // AGAPE_EDITOR_HIGHLIGHTERs_CARLO_H
