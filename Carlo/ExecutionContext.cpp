#include "Collections.h"
#include "ExecutionContext.h"
#include "String.h"
#include "Tuple.h"
#include "Value.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace Carlo
{

const char* ExecutionContext::s_errorMessages[] = {
    "None",
    "Unknown arithmetic operator",
    "No such function",
    "No such tuple or value",
    "No such tuple, value or function",
    "Unknown logical operator",
    "Tuple already exists",
    "Value already exists",
    "Value is not a list",
    "No such tuple",
    "Unable to save value"
};

ExecutionContext::ExecutionContext() :
  m_currentActor( nullptr ),
  m_stop( false )
{
}

ExecutionContext::~ExecutionContext()
{
    Map< String, Tuple* >::iterator tupleIt( m_tuples.begin() );
    for( ; tupleIt != m_tuples.end(); ++tupleIt )
    {
        if( tupleIt->first != m_receivedTupleAlias ) // Received tuple is on the stack, so don't delete() it!
        {
            delete( tupleIt->second );
        }
    }

    Map< String, Value* >::iterator valueIt( m_values.begin() );
    for( ; valueIt != m_values.end(); ++valueIt )
    {
        delete( valueIt->second );
    }
}

} // namespace Carlo

} // namespace Agape
