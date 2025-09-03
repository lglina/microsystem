#ifndef AGAPE_CARLO_LINDA2_H
#define AGAPE_CARLO_LINDA2_H

#include "Collections.h"

namespace Agape
{

namespace Linda2
{
namespace Actors
{
class Linda2Actor;
} // namespace Actors
} // namespace Linda2

class LiteStream;
class Value;

namespace Carlo
{

class ExecutionContext;
class Parser;
class ProgramManager;

class Linda2
{
    friend Parser;
    friend ProgramManager;

public:
    ~Linda2();

    void str( LiteStream& stream );

    // Evaluate the block for the first tuple handler for the first actor
    // of this program and returns the result - used in immediate mode.
    // Pre-requisite: value is nothing.
    bool evalOne( Value& value, ExecutionContext& executionContext );

private:
    Vector< Agape::Linda2::Actors::Linda2Actor* > m_actors;
};

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_LINDA2_LINDA2_H
