#ifndef AGAPE_CARLO_EXECUTION_CONTEXT_H
#define AGAPE_CARLO_EXECUTION_CONTEXT_H

#include "Collections.h"
#include "String.h"

namespace Agape
{

namespace Linda2
{
class Actor;
class Tuple;
} // namespace Linda2

using namespace Linda2;

class Value;

namespace Carlo
{

class ExecutionContext
{
public:
    enum ErrorCodes
    {
        errNone,
        errUnknownArithmeticOperator,
        errNoSuchFunction,
        errNoSuchTupleOrValue,
        errNoSuchTupleValueOrFunction,
        errUnknownLogicalOperator,
        errTupleAlreadyExists,
        errValueAlreadyExists,
        errValueIsNotAList,
        errNoSuchTuple,
        errUnableToSaveValue
    };

    struct RuntimeError
    {
        RuntimeError( int code, int line, int col, int len ) :
          m_code( code ),
          m_line( line ),
          m_col( col ),
          m_len( len )
        {
        }
        
        int m_code;
        int m_line;
        int m_col;
        int m_len;
    };

    static const char* s_errorMessages[];

    ExecutionContext();
    ~ExecutionContext();

    Actor* m_currentActor;
    String m_receivedTupleAlias;
    Map< String, Tuple* > m_tuples;
    Map< String, Value* > m_values;

    Vector< struct RuntimeError > m_runtimeErrors;

    bool m_stop;

private:
    // Make non-copyable.
    ExecutionContext( const ExecutionContext& other ) {};

    ExecutionContext& operator=( const ExecutionContext& other ) { ExecutionContext e; return e; };
};

} // namespace Carlo

} // namespace Agape

#endif // AGAPE_CARLO_EXECUTION_CONTEXT_H
