#include "Expressions/Expression.h"
#include "Utils/LiteStream.h"
#include "Block.h"
#include "EachStatement.h"
#include "ExecutionContext.h"
#include "String.h"
#include "Value.h"

namespace Agape
{

namespace Carlo
{

namespace Statements
{

Each::Each() :
  m_collection( nullptr ),
  m_block( nullptr )
{
}

Each::~Each()
{
    delete( m_collection );
    delete( m_block );
}

bool Each::eval( Value& value, ExecutionContext& executionContext )
{
    bool success( true );
    Value collection;
    if( m_collection->eval( collection, executionContext ) )
    {
        if( collection.type() == Value::list )
        {
            if( executionContext.m_values.find( m_elementName ) == executionContext.m_values.end() )
            {
                ConstListIterator it( collection.listBegin() );
                for( ; ( ( it != collection.listEnd() ) && !executionContext.m_stop ); ++it )
                {
                    executionContext.m_values[m_elementName] = *it;
                    m_block->eval( value, executionContext ); // Return value is result of eval() on block with last element of list.
                }

                executionContext.m_values.erase(m_elementName);
                executionContext.m_stop = false;
            }
            else
            {
                error( ExecutionContext::errValueAlreadyExists, executionContext );
                success = false;
            }
        }
        else if( collection.type() != Value::unknown ) // Empty lists will be type unknown...
        {
            error( ExecutionContext::errValueIsNotAList, executionContext );
            success = false;
        }
    }

    return success;
}

void Each::str( LiteStream& stream, int indent )
{
    strIndent( stream, indent );
    stream << "EachStatement\n";
    strIndent( stream, indent );
    stream << "{\n";
    strIndent( stream, indent + 4 );
    stream << "Element name: " << m_elementName << "\n";
    strIndent( stream, indent + 4 );
    stream << "Collection: ";
    m_collection->str( stream, indent + 4 );
    stream << "Block: ";
    m_block->str( stream, indent + 4 );
    strIndent( stream, indent );
    stream << "}\n";
}

} // namespace Statements

} // namespace Carlo

} // namespace Agape
