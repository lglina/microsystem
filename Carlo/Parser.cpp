#include "Actors/Actor.h"
#include "Actors/Linda2Actor.h"
#include "Expressions/ArithmeticExpression.h"
#include "Expressions/ComparisonExpression.h"
#include "Expressions/Expression.h"
#include "Expressions/FunctionExpression.h"
#include "Expressions/IdentifierExpression.h"
#include "Expressions/LogicalExpression.h"
#include "Highlighters/Highlighter.h"
#include "Loggers/Logger.h"
#include "Statements/CreatesStatement.h"
#include "Statements/EachStatement.h"
#include "Statements/IfStatement.h"
#include "Statements/MakesStatement.h"
#include "Statements/SendsStatement.h"
#include "Statements/Statement.h"
#include "Statements/StopStatement.h"
#include "Statements/WhileStatement.h"
#include "Block.h"
#include "Collections.h"
#include "FunctionDispatcher.h"
#include "Terminal.h"
#include "TupleHandler.h"
#include "TupleRouter.h"
#include "Tuple.h"
#include "Value.h"

#include "Lexer.h"
#include "Parser.h"

using namespace Agape::Linda2;
using Agape::Terminal;

namespace
{
    const int hlNone( Terminal::colGrey );
    const int hlError( Terminal::colYellow );
    const int hlCarlo( Terminal::colCyan );
    const int hlLinda( Terminal::colMagenta );
    const int hlNote( Terminal::colBrown );
    const int hlTupleKey( Terminal::colWhite );
    const int hlArgumentName( Terminal::colWhite );
} // Anonymous namespace

namespace Agape
{

namespace Carlo
{

const char* Parser::s_errorMessages[] = {
    "None",
    "Expected \"actor\"",
    "Expected actor name",
    "Expected end of line",
    "Expected \"receives\"",
    "Expected tuple type",
    "Expected source actor name",
    "Expected tuple alias",
    "Expected \"end\"",
    "Expected statement",
    "Expected tuple type or tuple name",
    "Expected value name",
    "Expected \"tuple\" or tuple type",
    "Expected tuple name",
    "Expected \"tuple\" or \"value\"",
    "Expected array element name",
    "Expected \"in\"",
    "Expected closing ')'",
    "Expected expression",
    "Expected \"than\"",
    "Duplicate argument name",
    "Expected argument name"
};

// FIXME: Have destructors for everything under Linda2, so it can
// be delete()'d, and everything inside will be deallocated.

// FIXME: Perhaps we really should encapsulate memory management in
// AST objects, rather than having Parser as a friend and having
// it do all the allocations...

Parser::Parser( Deque< struct Lexer::Token >& lexerTokens,
                TupleRouter& tupleRouter,
                FunctionDispatcher& functionDispatcher,
                bool buildTree ) :
  m_lexerTokens( lexerTokens ),
  m_tupleRouter( tupleRouter ),
  m_functionDispatcher( functionDispatcher ),
  m_buildTree( buildTree ),
  m_errors( nullptr )
{
}

bool Parser::parse( Linda2*& linda2,
                    Vector< struct Highlighter::Token >* highlights,
                    Vector< Error >* errors )
{
    if( m_buildTree )
    {
        linda2 = new Linda2;
    }

    m_highlights = highlights;
    m_errors = errors;

    bool success( true );

    bool firstActor( true );

    while( !isNextCode( Lexer::l_none ) )
    {
        if( isNextCode( Lexer::l_actor ) )
        {
            Lexer::Token initialToken( nextToken() );
            highlight( hlCarlo );
            eatToken();

            Actors::Linda2Actor* _actor( nullptr );
            if( actor( _actor, firstActor ) )
            {
                if( m_buildTree )
                {
                    _actor->initialToken( initialToken );
                    linda2->m_actors.push_back( _actor );
                }
                firstActor = false;
            }
            else
            {
                success = false;
            }
        }
        else if( isNextCode( Lexer::l_note ) )
        {
            highlight( hlNote );
            eatToken();
        }
        else if( isNextCode( Lexer::l_eol ) )
        {
            eatToken();
        }
        else
        {
            addError( errExpectedActor );
            eatToken();
            success = false;
        }
    }

    if( m_buildTree && !success )
    {
        delete( linda2 );
    }

    return success;
}

bool Parser::isNextCode( enum Lexer::Code code )
{
    if( m_lexerTokens.empty() && ( code == Lexer::l_none ) )
    {
        return true;
    }
    else if( m_lexerTokens.empty() )
    {
        return false;
    }
    else
    {
        return( m_lexerTokens.front().m_code == code );
    }
}

struct Lexer::Token Parser::nextToken()
{
    struct Lexer::Token token;
    if( !m_lexerTokens.empty() )
    {
        return( m_lexerTokens.front() );
    }

    return token;
}

void Parser::eatToken()
{
    if( !m_lexerTokens.empty() )
    {
        if( m_highlights )
        {
            // If no explicit highlight has been created for this token,
            // create a "none" highlight.
            Lexer::Token token( nextToken() );
            if( m_highlights->empty() ||
                m_highlights->back().m_line != token.m_line || 
                m_highlights->back().m_column != token.m_column )
            {
                highlight( hlNone );
            }
        }

        m_lexerTokens.pop_front();
    }
}

void Parser::highlight( int attributes )
{
    if( m_highlights )
    {
        Lexer::Token token( nextToken() );
        if( ( token.m_code != Lexer::l_none ) &&
            ( token.m_len > 0 ) )
        {
            Highlighter::Token highlight;
            highlight.m_line = token.m_line;
            highlight.m_column = token.m_column;
            highlight.m_length = token.m_len;
            highlight.m_attributes = attributes;
            m_highlights->push_back( highlight );
        }
    }
}

void Parser::addError( int errorCode )
{
    if( m_errors )
    {
        Lexer::Token token( nextToken() );
        if( token.m_code != Lexer::l_none )
        {
            highlight( hlError );
            Error error;
            error.m_errorCode = errorCode;
            error.m_line = token.m_line;
            error.m_column = token.m_column;
            error.m_lexerCode = token.m_code;
            m_errors->push_back( error );
        }
    }
}

bool Parser::actor( Actors::Linda2Actor*& _actor, bool firstActor )
{
#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Parsing actor" );
#endif

    bool success( true );

    // Actor definition.
    // First actor doesn't need an explicit name, as it will be automatically
    // named using the snowflake of the associated object on load
    // by ProgramManager.
    String name;
    if( isNextCode( Lexer::l_string ) )
    {
        name = nextToken().m_string;
        eatToken();
    }
    else if( !firstActor )
    {
        addError( errExpectedActorName );
        eatToken();
        success = false;
    }

    if( isNextCode( Lexer::l_eol ) )
    {
        eatToken();
    }
    else
    {
        addError( errExpectedEOL );
        eatToken();
        success = false;
    }

    // Tuple handlers (0..N).
    if( m_buildTree )
    {
        _actor = new Actors::Linda2Actor( name, m_tupleRouter );
    }

    while( !isNextCode( Lexer::l_none ) )
    {
        if( isNextCode( Lexer::l_receives ) )
        {
            Lexer::Token initialToken( nextToken() );
            highlight( hlLinda );
            eatToken();

            TupleHandler* _tupleHandler( nullptr );
            if( tupleHandler( _tupleHandler ) )
            {
                if( m_buildTree )
                {
                    _tupleHandler->initialToken( initialToken );
                    _actor->m_tupleHandlers.push_back( _tupleHandler );
                }
            }
            else
            {
                success = false;
            }
        }
        else if( isNextCode( Lexer::l_note ) )
        {
            highlight( hlNote );
            eatToken();
        }
        else if( isNextCode( Lexer::l_eol ) )
        {
            eatToken();
        }
        else if( isNextCode( Lexer::l_end ) )
        {
            highlight( hlCarlo );
            eatToken();
            break;
        }
        else
        {
            addError( errExpectedReceives );
            eatToken();
            success = false;
        }
    }

    if( m_buildTree && !success )
    {
        delete( _actor );
    }

#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Actor end" );
#endif
    return success;
}

bool Parser::tupleHandler( TupleHandler*& _tupleHandler )
{
    if( m_buildTree )
    {
        _tupleHandler = new TupleHandler;
    }

    String sourceActor;
    String tupleType;
    String tupleAlias;

#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Parsing tuple handler" );
#endif

    bool success( true );

    if( isNextCode( Lexer::l_string ) )
    {
        tupleType = nextToken().m_string;
        eatToken();
    }
    else
    {
        addError( errExpectedTupleType );
        eatToken();
        success = false;
    }

    // Optional source actor
    if( isNextCode( Lexer::l_from ) )
    {
        highlight( hlLinda );
        eatToken();

        if( isNextCode( Lexer::l_string ) )
        {
            sourceActor = nextToken().m_string;
            eatToken();
        }
        else
        {
            addError( errExpectedSourceActorName );
            eatToken();
            success = false;
        }
    }

    // Optional alias for tuple
    if( isNextCode( Lexer::l_as ) )
    {
        highlight( hlLinda );
        eatToken();

        if( isNextCode( Lexer::l_string ) )
        {
            tupleAlias = nextToken().m_string;
            eatToken();
        }
        else
        {
            addError( errExpectedTupleAlias );
            eatToken();
            success = false;
        }
    }

    // Optional matching condition
    if( isNextCode( Lexer::l_where ) )
    {
        highlight( hlLinda );
        eatToken();

        Expression* _expression( nullptr );
        Lexer::Token initialToken( nextToken() );
        if( expression( _expression, 0 ) )
        {
            if( m_buildTree )
            {
                _expression->initialToken( initialToken );
                _tupleHandler->m_condition = _expression;
            }
        }
        else
        {
            success = false;
        }
    }

    if( isNextCode( Lexer::l_eol ) )
    {
        eatToken();
    }
    else
    {
        addError( errExpectedEOL );
        eatToken();
        success = false;
    }

    // FIXME: To avoid needing to create the whole abstract parse tree
    // in memory, we could stop parsing at this level of the tree and
    // defer parsing of blocks until (/if) they're called.
    Block* _block( nullptr );
    Lexer::Token initialToken( nextToken() );
    if( block( _block ) )
    {
        if( m_buildTree )
        {
            _block->initialToken( initialToken );
        }
    }
    else
    {
        success = false;
    }

    if( isNextCode( Lexer::l_end ) )
    {
        highlight( hlCarlo );
        eatToken();
    }
    else
    {
        addError( errExpectedEnd );
        eatToken();
        success = false;
    }

    if( m_buildTree && success )
    {
        _tupleHandler->m_sourceActor = sourceActor;
        _tupleHandler->m_tupleType = tupleType;
        _tupleHandler->m_tupleAlias = tupleAlias;
        _tupleHandler->m_block = _block;
        // FIXME: The TupleHandler should send a routing request here.
        // (Have the TupleHandler do this internally, in the same way that
        // the constructor for Linda2Actor registers that actor).
    }
    else if( m_buildTree && !success )
    {
        delete( _tupleHandler );
    }

#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Handler end" );
#endif

    return success;
}

bool Parser::block( Block*& _block )
{
    if( m_buildTree )
    {
        _block = new Block;
    }

#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Parsing block" );
#endif

    bool success( true );

    while( !isNextCode( Lexer::l_end ) &&
           !isNextCode( Lexer::l_else ) &&
           !isNextCode( Lexer::l_none ) ) // End and else are block terminators
    {
        if( isNextCode( Lexer::l_sends ) )
        {
            Lexer::Token initialToken( nextToken() );
            highlight( hlLinda );
            eatToken();

            Statements::Sends* _statement( nullptr );
            if( sendsStatement( _statement ) )
            {
                if( m_buildTree )
                {
                    _statement->initialToken( initialToken );
                    _block->m_statements.push_back( _statement );
                }
            }
            else
            {
                success = false;
            }
        }
        else if( isNextCode( Lexer::l_if ) )
        {
            Lexer::Token initialToken( nextToken() );
            highlight( hlCarlo );
            eatToken();

            Statements::If* _statement( nullptr );
            if( ifStatement( _statement ) )
            {
                if( m_buildTree )
                {
                    _statement->initialToken( initialToken );
                    _block->m_statements.push_back( _statement );
                }
            }
            else
            {
                success = false;
            }
        }
        else if( isNextCode( Lexer::l_makes ) )
        {
            Lexer::Token initialToken( nextToken() );
            highlight( hlCarlo );
            eatToken();

            Statements::Makes* _statement( nullptr );
            if( makesStatement( _statement ) )
            {
                if( m_buildTree )
                {
                    _statement->initialToken( initialToken );
                    _block->m_statements.push_back( _statement );
                }
            }
            else
            {
                success = false;
            }
        }
        else if( isNextCode( Lexer::l_creates ) )
        {
            Lexer::Token initialToken( nextToken() );
            highlight( hlCarlo );
            eatToken();

            Statements::Creates* _statement( nullptr );
            if( createsStatement( _statement ) )
            {
                if( m_buildTree )
                {
                    _statement->initialToken( initialToken );
                    _block->m_statements.push_back( _statement );
                }
            }
            else
            {
                success = false;
            }
        }
        else if( isNextCode( Lexer::l_each ) )
        {
            Lexer::Token initialToken( nextToken() );
            highlight( hlCarlo );
            eatToken();

            Statements::Each* _statement( nullptr );
            if( eachStatement( _statement ) )
            {
                if( m_buildTree )
                {
                    _statement->initialToken( initialToken );
                    _block->m_statements.push_back( _statement );
                }
            }
            else
            {
                success = false;
            }
        }
        else if( isNextCode( Lexer::l_while ) )
        {
            Lexer::Token initialToken( nextToken() );
            highlight( hlCarlo );
            eatToken();

            Statements::While* _statement( nullptr );
            if( whileStatement( _statement ) )
            {
                if( m_buildTree )
                {
                    _statement->initialToken( initialToken );
                    _block->m_statements.push_back( _statement );
                }
            }
            else
            {
                success = false;
            }
        }
        else if( isNextCode( Lexer::l_stop ) )
        {
            Lexer::Token initialToken( nextToken() );
            highlight( hlCarlo );
            eatToken();

            if( m_buildTree )
            {
                Statements::Stop* _statement( new Statements::Stop );
                _statement->initialToken( initialToken );
                _block->m_statements.push_back( _statement );
            }
        }
        else if( isNextCode( Lexer::l_note ) )
        {
            highlight( hlNote );
            eatToken();
        }
        else if( isNextCode( Lexer::l_eol ) )
        {
            eatToken();
        }
        else
        {
            // Invalid code.
            addError( errExpectedStatement );
            eatToken();
            success = false;
        }
    }

    if( m_buildTree && !success )
    {
        delete( _block );
    }

#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Block end" );
#endif

    return success;
}

bool Parser::sendsStatement( Statements::Sends*& _sendsStatement )
{
    String destinationActor;
    String destinationID;

#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Parsing send statement" );
#endif

    bool success( true );

    if( m_buildTree )
    {
        _sendsStatement = new Statements::Sends( m_tupleRouter );
    }

    // Optional destination actor (if not already defined within tuple)
    if( isNextCode( Lexer::l_to ) )
    {
        highlight( hlLinda );
        eatToken();

        Expression* _expression( nullptr );
        Lexer::Token initialToken( nextToken() );
        if( expression( _expression, 0 ) )
        {
            if( m_buildTree )
            {
                _expression->initialToken( initialToken );
                _sendsStatement->m_destinationActor = _expression;
            }
        }
        else
        {
            success = false;
        }
    }

    // Optional destination machine (if not already defined within tuple)
    if( isNextCode( Lexer::l_on ) )
    {
        highlight( hlLinda );
        eatToken();

        Expression* _expression( nullptr );
        Lexer::Token initialToken( nextToken() );
        if( expression( _expression, 0 ) )
        {
            if( m_buildTree )
            {
                _expression->initialToken( initialToken );
                _sendsStatement->m_destinationID = _expression;
            }
        }
        else
        {
            success = false;
        }
    }

    if( isNextCode( Lexer::l_tuple ) )
    {
        // Send named tuple
        highlight( hlLinda );
        eatToken();

        if( isNextCode( Lexer::l_string ) )
        {
            String tupleName = nextToken().m_string;
            eatToken();

            if( m_buildTree )
            {
                _sendsStatement->m_type = Statements::Sends::t_namedTuple;
                _sendsStatement->m_tupleName = tupleName;
            }
        }
        else
        {
            addError( errExpectedTupleName );
            eatToken();
            success = false;
        }
    }
    else if( isNextCode( Lexer::l_string ) )
    {
        String tupleType = nextToken().m_string;
        eatToken();

        if( m_buildTree )
        {
            _sendsStatement->m_type = Statements::Sends::t_tupleLiteral;
            _sendsStatement->m_tupleType = tupleType;
        }

        while( !isNextCode( Lexer::l_eol ) &&
               !isNextCode( Lexer::l_none ) )
        {
            String valueName;
            if( isNextCode( Lexer::l_string ) )
            {
                valueName = nextToken().m_string;
                highlight( hlTupleKey );
                eatToken();
            }
            else
            {
                addError( errExpectedValueName );
                eatToken();
                success = false;
            }
            
            Expression* _expression( nullptr );
            Lexer::Token initialToken( nextToken() );
            if( expression( _expression, 0 ) )
            {
                if( m_buildTree )
                {
                    _expression->initialToken( initialToken );
                    _sendsStatement->m_tupleValues[valueName] = _expression;
                }
            }
            else
            {
                success = false;
            }
        }
    }
    else
    {
        addError( errExpectedTupleOrTupleType );
        eatToken();
        success = false;
    }

    if( m_buildTree && !success )
    {
        delete( _sendsStatement );
    }

    return success;
}

bool Parser::ifStatement( Statements::If*& _ifStatement )
{
    if( m_buildTree )
    {
        _ifStatement = new Statements::If;
    }

#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Parsing if statement" );
#endif

    bool success( true );

    Expression* _expression( nullptr );
    Lexer::Token initialToken( nextToken() );
    if( expression( _expression, 0 ) )
    {
        if( m_buildTree )
        {
            _expression->initialToken( initialToken );
            _ifStatement->m_condition = _expression;
        }
    }
    else
    {
        success = false;
    }

    if( isNextCode( Lexer::l_eol ) )
    {
        eatToken();
    }
    else
    {
        addError( errExpectedEOL );
        eatToken();
        success = false;
    }

    Block* _block( nullptr );
    initialToken = nextToken();
    if( block( _block ) )
    {
        if( m_buildTree )
        {
            _block->initialToken( initialToken );
            _ifStatement->m_block = _block;
        }
    }
    else
    {
        success = false;
    }

    if( isNextCode( Lexer::l_else ) )
    {
        highlight( hlCarlo );
        eatToken(); // Eat else

        if( isNextCode( Lexer::l_eol ) )
        {
            eatToken();
        }
        else
        {
            addError( errExpectedEOL );
            eatToken();
            success = false;
        }

        initialToken = nextToken();
        if( block( _block ) )
        {
            if( m_buildTree )
            {
                _block->initialToken( initialToken );
                _ifStatement->m_elseBlock = _block;
            }
        }
        else
        {
            success = false;
        }
    }

    if( isNextCode( Lexer::l_end ) )
    {
        highlight( hlCarlo );
        eatToken();
    }
    else
    {
        addError( errExpectedEnd );
        eatToken();
        success = false;
    }

    if( m_buildTree && !success )
    {
        delete( _ifStatement );
    }

    return success;
}

bool Parser::makesStatement( Statements::Makes*& _makesStatement )
{
    if( m_buildTree )
    {
        _makesStatement = new Statements::Makes;
    }

#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Parsing makes statement" );
#endif

    bool success( true );

    if( isNextCode( Lexer::l_string ) )
    {
        if( m_buildTree )
        {
            Expressions::Identifier* identifierExpression( new Expressions::Identifier( m_functionDispatcher ) );
            identifierExpression->m_string = nextToken().m_string;
            identifierExpression->initialToken( nextToken() );
            _makesStatement->m_lhs = identifierExpression;
        }
        eatToken();
    }
    else
    {
        addError( errExpectedValueName );
        eatToken();
        success = false;
    }

    Expression* _expression( nullptr );
    Lexer::Token initialToken( nextToken() );
    if( expression( _expression, 0 ) )
    {
        if( m_buildTree )
        {
            _expression->initialToken( initialToken );
            _makesStatement->m_rhs = _expression;
        }
    }
    else
    {
        success = false;
    }

    if( m_buildTree && !success )
    {
        delete( _makesStatement );
    }

    return success;
}

bool Parser::createsStatement( Statements::Creates*& _createsStatement )
{
    if( m_buildTree )
    {
        _createsStatement = new Statements::Creates;
    }

#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Parsing creates statement" );
#endif

    bool success( true );

    if( isNextCode( Lexer::l_tuple ) )
    {
        eatToken();

        String tupleName;
        String tupleType;

        if( isNextCode( Lexer::l_string ) )
        {
            tupleName = nextToken().m_string;
            eatToken();
        }
        else
        {
            addError( errExpectedTupleName );
            eatToken();
            success = false;
        }

        if( isNextCode( Lexer::l_string ) )
        {
            tupleType = nextToken().m_string;
            eatToken();
        }
        else
        {
            addError( errExpectedTupleType );
            eatToken();
            success = false;
        }

        if( m_buildTree )
        {
            _createsStatement->m_type = Statements::Creates::t_tuple;
            _createsStatement->m_tupleName = tupleName;
            _createsStatement->m_tupleType = tupleType;
        }

        while( !isNextCode( Lexer::l_eol ) &&
               !isNextCode( Lexer::l_none ) )
        {
            String valueName;
            if( isNextCode( Lexer::l_string ) )
            {
                valueName = nextToken().m_string;
                eatToken();
            }
            else
            {
                addError( errExpectedValueName );
                eatToken();
                success = false;
            }
            
            Expression* _expression( nullptr );
            Lexer::Token initialToken( nextToken() );
            if( expression( _expression, 0 ) )
            {
                if( m_buildTree )
                {
                    _expression->initialToken( initialToken );
                    _createsStatement->m_tupleValues[valueName] = _expression;
                }
            }
            else
            {
                success = false;
            }
        }
    }
    else if( isNextCode( Lexer::l_value ) )
    {
        eatToken();

        String valueName;
        if( isNextCode( Lexer::l_string ) )
        {
            valueName = nextToken().m_string;
            eatToken();
        }
        else
        {
            addError( errExpectedValueName );
            eatToken();
            success = false;
        }

        if( m_buildTree )
        {
            _createsStatement->m_type = Statements::Creates::t_value;
            _createsStatement->m_valueName = valueName;
        }

        // Optional initialiser
        if( !isNextCode( Lexer::l_eol ) )
        {
            Expression* _expression( nullptr );
            Lexer::Token initialToken( nextToken() );
            if( expression( _expression, 0 ) )
            {
                if( m_buildTree )
                {
                    _expression->initialToken( initialToken );
                    _createsStatement->m_valueExpression = _expression;
                }
            }
            else
            {
                success = false;
            }
        }
    }
    else
    {
        addError( errExpectedTupleOrValue );
        eatToken();
        success = false;
    }

    if( m_buildTree && !success )
    {
        delete( _createsStatement );
    }

    return success;
}

bool Parser::eachStatement( Statements::Each*& _eachStatement )
{
    if( m_buildTree )
    {
        _eachStatement = new Statements::Each;
    }

#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Parsing each statement" );
#endif

    bool success( true );

    // Element name
    if( isNextCode( Lexer::l_string ) )
    {
        if( m_buildTree )
        {
            _eachStatement->m_elementName = nextToken().m_string;
        }
        eatToken();
    }
    else
    {
        addError( errExpectedArrayElementName );
        eatToken();
        success = false;
    }

    // "in"
    if( isNextCode( Lexer::l_in ) )
    {
        highlight( hlCarlo );
        eatToken();
    }
    else
    {
        addError( errExpectedIn );
        eatToken();
        success = false;
    }

    // Array value name
    if( isNextCode( Lexer::l_string ) )
    {
        if( m_buildTree )
        {
            Expressions::Identifier* identifierExpression( new Expressions::Identifier( m_functionDispatcher ) );
            identifierExpression->m_string = nextToken().m_string;
            identifierExpression->initialToken( nextToken() );
            _eachStatement->m_collection = identifierExpression;
        }
        eatToken();
    }
    else
    {
        addError( errExpectedValueName );
        eatToken();
        success = false;
    }

    // EOL
    if( isNextCode( Lexer::l_eol ) )
    {
        eatToken();
    }
    else
    {
        addError( errExpectedEOL );
        eatToken();
        success = false;
    }

    // Loop block
    Block* _block( nullptr );
    Lexer::Token initialToken = nextToken();
    if( block( _block ) )
    {
        if( m_buildTree )
        {
            _block->initialToken( initialToken );
            _eachStatement->m_block = _block;
        }
    }
    else
    {
        success = false;
    }

    if( isNextCode( Lexer::l_end ) )
    {
        highlight( hlCarlo );
        eatToken();
    }
    else
    {
        addError( errExpectedEnd );
        eatToken();
        success = false;
    }

    if( m_buildTree && !success )
    {
        delete( _eachStatement );
    }

    return success;
}

bool Parser::whileStatement( Statements::While*& _whileStatement )
{
    if( m_buildTree )
    {
        _whileStatement = new Statements::While;
    }

#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Parsing while statement" );
#endif

    bool success( true );

    Expression* _expression( nullptr );
    Lexer::Token initialToken( nextToken() );
    if( expression( _expression, 0 ) )
    {
        if( m_buildTree )
        {
            _expression->initialToken( initialToken );
            _whileStatement->m_condition = _expression;
        }
    }
    else
    {
        success = false;
    }

    if( isNextCode( Lexer::l_eol ) )
    {
        eatToken();
    }
    else
    {
        addError( errExpectedEOL );
        eatToken();
        success = false;
    }

    Block* _block( nullptr );
    initialToken = nextToken();
    if( block( _block ) )
    {
        if( m_buildTree )
        {
            _block->initialToken( initialToken );
            _whileStatement->m_block = _block;
        }
    }
    else
    {
        success = false;
    }

    if( isNextCode( Lexer::l_end ) )
    {
        highlight( hlCarlo );
        eatToken();
    }
    else
    {
        addError( errExpectedEnd );
        eatToken();
        success = false;
    }

    if( m_buildTree && !success )
    {
        delete( _whileStatement );
    }

    return success;
}

bool Parser::expression( Expression*& value, int precedence )
{
    // FIXME: Implement prefixes.
    // FIXME: Ensure we eat something here.
    String functionName;
    Lexer::Token functionInitialToken;

#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Parsing value expression" );
#endif

    bool success( true );

    if( isNextCode( Lexer::l_string ) || isNextCode( Lexer::l_quotedString ) )
    {
#ifdef LOG_CARLO_PARSER
        LOG_DEBUG( "Found identifier (string)" );
#endif
        String str( nextToken().m_string );
        bool quotedString( isNextCode( Lexer::l_quotedString ) );
        Lexer::Token initialToken( nextToken() );
        eatToken();
        if( !isNextCode( Lexer::l_with ) )
        {
            if( m_buildTree )
            {
                Expressions::Identifier* identifier( new Expressions::Identifier( m_functionDispatcher ) );
                identifier->m_string = str;
                identifier->m_quotedString = quotedString;
                identifier->initialToken( initialToken );
                value = identifier;
            }
        }
        else if( !quotedString )
        {
            // This is a function name string literal.
            functionName = str;
            functionInitialToken = initialToken;
        }
        else
        {
            // Function name cannot be a quoted string literal.
            addError( errExpectedExpression );
            success = false;
        }
    }
    else if( isNextCode( Lexer::l_float ) )
    {
#ifdef LOG_CARLO_PARSER
        LOG_DEBUG( "Found identifier (float)" );
#endif
        if( m_buildTree )
        {
            Expressions::Identifier* identifier( new Expressions::Identifier( m_functionDispatcher ) );
            identifier->m_float = nextToken().m_float;
            identifier->initialToken( nextToken() );
            value = identifier;
        }
        eatToken();
    }
    else if( isNextCode( Lexer::l_leftParen ) )
    {
#ifdef LOG_CARLO_PARSER
        LOG_DEBUG( "Found left parenthesis" );
#endif
        highlight( hlCarlo );
        eatToken();
        if( expression( value, 0 ) )
        {
            if( isNextCode( Lexer::l_rightParen ) )
            {
                highlight( hlCarlo );
                eatToken();
            }
            else
            {
                addError( errExpectedClosingParenthesis );
                success = false;
            }
        }
        else
        {
            // Expression error.
            if( isNextCode( Lexer::l_rightParen ) )
            {
                eatToken();
            }
            success = false;
        }
    }
    else
    {
        addError( errExpectedExpression );
        eatToken();
        success = false;
    }

    // Infix.
    while( precedence < nextTokenPrecedence() )
    {
        if( isNextCode( Lexer::l_is ) ||
            isNextCode( Lexer::l_more ) ||
            isNextCode( Lexer::l_less ) )
        {
#ifdef LOG_CARLO_PARSER
            LOG_DEBUG( "Found comparison operator" );
#endif
            enum Expressions::Comparison::Operation operation( Expressions::Comparison::unknown );
            switch( nextToken().m_code )
            {
            case Lexer::l_is:
                operation = Expressions::Comparison::equality;
                break;
            case Lexer::l_more:
                operation = Expressions::Comparison::moreThan;
                break;
            case Lexer::l_less:
                operation = Expressions::Comparison::lessThan;
                break;
            default:
                break;
            }
            
            int thisPrecedence( nextTokenPrecedence() );
            highlight( hlCarlo );
            eatToken();

            switch( operation )
            {
            case Expressions::Comparison::equality:
                if( isNextCode( Lexer::l_not ) )
                {
                    highlight( hlCarlo );
                    eatToken();
                    operation = Expressions::Comparison::inequality;
                }
                break;
            case Expressions::Comparison::moreThan:
            case Expressions::Comparison::lessThan:
                if( isNextCode( Lexer::l_than ) )
                {
                    highlight( hlCarlo );
                    eatToken();
                }
                else
                {
                    addError( errExpectedThan );
                    success = false;
                }
                break;
            default:
                break;
            }

            Expressions::Comparison* comparisonExpression( new Expressions::Comparison );
            Lexer::Token initialToken( nextToken() );
            if( expression( comparisonExpression->m_rhs, thisPrecedence ) )
            {
                if( m_buildTree )
                {
                    // N.B. Always set m_lhs to point to value at the same we
                    // set value to point to the expression itself, to prevent
                    // having two pointers to the same memory.
                    comparisonExpression->m_lhs = value;
                    comparisonExpression->m_operation = operation;
                    comparisonExpression->m_rhs->initialToken( initialToken );
                    value = comparisonExpression;
                }
                else
                {
                    delete( comparisonExpression );
                }
            }
            else
            {
                // Expression error.
                delete( comparisonExpression ); // Avoid memory leak.
                success = false;
            }
        }
        else if( isNextCode( Lexer::l_and ) ||
                 isNextCode( Lexer::l_or ) )
        {
#ifdef LOG_CARLO_PARSER
            LOG_DEBUG( "Found logical operator" );
#endif
            enum Expressions::Logical::Operation operation( Expressions::Logical::unknown );
            switch( nextToken().m_code )
            {
            case Lexer::l_and:
                operation = Expressions::Logical::logicalAnd;
                break;
            case Lexer::l_or:
                operation = Expressions::Logical::logicalOr;
                break;
            default:
                break;
            }

            int thisPrecedence( nextTokenPrecedence() );
            highlight( hlCarlo );
            eatToken();

            Expressions::Logical* logicalExpression( new Expressions::Logical );
            Lexer::Token initialToken( nextToken() );
            if( expression( logicalExpression->m_rhs, thisPrecedence ) )
            {
                if( m_buildTree )
                {
                    logicalExpression->m_lhs = value;
                    logicalExpression->m_operation = operation;
                    logicalExpression->m_rhs->initialToken( initialToken );
                    value = logicalExpression;
                }
                else
                {
                    delete( logicalExpression );
                }
            }
            else
            {
                // Expression error.
                delete( logicalExpression ); // Avoid memory leak.
                success = false;
            }
        }
        else if( isNextCode( Lexer::l_plus ) ||
                 isNextCode( Lexer::l_minus ) ||
                 isNextCode( Lexer::l_asterisk ) ||
                 isNextCode( Lexer::l_forwardSlash ) ||
                 isNextCode( Lexer::l_percent ) )
        {
#ifdef LOG_CARLO_PARSER
            LOG_DEBUG( "Found arithmetic operator" );
#endif
            enum Expressions::Arithmetic::Operation operation( Expressions::Arithmetic::unknown );
            switch( nextToken().m_code )
            {
            case Lexer::l_plus:
                operation = Expressions::Arithmetic::addition;
                break;
            case Lexer::l_minus:
                operation = Expressions::Arithmetic::subtraction;
                break;
            case Lexer::l_asterisk:
                operation = Expressions::Arithmetic::multiplication;
                break;
            case Lexer::l_forwardSlash:
                operation = Expressions::Arithmetic::division;
                break;
            case Lexer::l_percent:
                operation = Expressions::Arithmetic::modulo;
                break;
            default: // Can't happen.
                break;
            }

            int thisPrecedence( nextTokenPrecedence() );
            highlight( hlCarlo );
            eatToken();
            
            Expressions::Arithmetic* arithmeticExpression( new Expressions::Arithmetic );
            Lexer::Token initialToken( nextToken() );
            if( expression( arithmeticExpression->m_rhs, thisPrecedence ) )
            {
                if( m_buildTree )
                {
                    arithmeticExpression->m_lhs = value;
                    arithmeticExpression->m_operation = operation;
                    arithmeticExpression->m_rhs->initialToken( initialToken );
                    value = arithmeticExpression;
                }
                else
                {
                    delete( arithmeticExpression );
                }
            }
            else
            {
                // Expression error.
                delete( arithmeticExpression ); // Avoid memory leak.
                success = false;
            }
        }
        else if( isNextCode( Lexer::l_with ) )
        {
            highlight( hlCarlo );
            eatToken();
            Expressions::Function* functionExpression( new Expressions::Function( m_functionDispatcher ) );
            functionExpression->m_name = functionName;
            functionExpression->initialToken( functionInitialToken );

            while( success &&
                  ( functionExpression->m_argumentExpressions.empty() ||
                    isNextCode( Lexer::l_string ) ) )
            {
                // Look for argument name
                String argumentName;
                if( isNextCode( Lexer::l_string ) )
                {
                    argumentName = nextToken().m_string;
                    highlight( hlArgumentName );
                    eatToken();

                    if( functionExpression->m_argumentExpressions.find( argumentName ) != functionExpression->m_argumentExpressions.end() )
                    {
                        addError( errDuplicateArgumentName );
                        success = false;
                    }
                }
                else
                {
                    addError( errExpectedArgumentName );
                    eatToken();
                    success = false;
                }

                if( success )
                {
                    Lexer::Token initialToken( nextToken() );
                    if( expression( functionExpression->m_argumentExpressions[argumentName], 0 ) )
                    {
                        if( m_buildTree )
                        {
                            functionExpression->m_argumentExpressions[argumentName]->initialToken( initialToken );
                        }
                    }
                    else
                    {
                        success = false;
                    }
                }
            }

            if( success && m_buildTree )
            {
                value = functionExpression;
            }
            else
            {
                delete( functionExpression ); // Avoid memory leak.
            }
        }
        else
        {
            eatToken();
            success = false;
        }
    }

#ifdef LOG_CARLO_PARSER
    if( ( precedence >= nextTokenPrecedence() ) && ( nextTokenPrecedence() > 0 ) )
    {
        LOG_DEBUG( "Next operator has higher precedence. Backing out." );
    }
#endif

    if( m_buildTree && !success )
    {
        delete( value );
        // value is a pointer provided by the caller and could be inside a
        // larger object that the caller will itself delete() (e.g. an m_rhs
        // pointer inside a LogicalExpression and we're being called
        // recursively). Set to nullptr here so the caller will not attempt
        // to delete the pointed-to memory a second time.
        value = nullptr;
    }

#ifdef LOG_CARLO_PARSER
    LOG_DEBUG( "Returning value" );
#endif
    return success;
}

int Parser::nextTokenPrecedence()
{
    if( isNextCode( Lexer::l_and ) || isNextCode( Lexer::l_or ) )
    {
        return 1;
    }
    else if( isNextCode( Lexer::l_is ) || isNextCode( Lexer::l_more ) || isNextCode( Lexer::l_less ) )
    {
        return 2;
    }
    else if( isNextCode( Lexer::l_plus ) || isNextCode( Lexer::l_minus ) )
    {
        return 3;
    }
    else if( isNextCode( Lexer::l_asterisk ) || isNextCode( Lexer::l_forwardSlash ) || isNextCode( Lexer::l_percent ) )
    {
        return 4;
    }
    else if( isNextCode( Lexer::l_leftParen ) )
    {
        return 5;
    }
    else if( isNextCode( Lexer::l_with ) )
    {
        return 6;
    }

    return 0;
}

} // namespace Linda2

} // namespace Agape
