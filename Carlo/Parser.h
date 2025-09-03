#ifndef AGAPE_CARLO_PARSER_H
#define AGAPE_CARLO_PARSER_H

#include "Highlighters/Highlighter.h"
#include "Collections.h"
#include "Lexer.h"
#include "Linda2.h"
#include "String.h"

using namespace Agape::Linda2;
using namespace Agape::Editor;

namespace Agape
{

class Value;

namespace Linda2
{

namespace Actors
{
class Linda2Actor;
} // namespace Actors

class TupleHandler;
class TupleRouter;
} // namespace Linda2

namespace Carlo
{

namespace Statements
{
class Creates;
class Each;
class If;
class Makes;
class Sends;
class While;
} // namespace Statements

class Block;
class Expression;
class FunctionDispatcher;
class Statement;

class Parser
{
public:
    struct Error
    {
        Error() :
          m_errorCode( -1 ),
          m_line( 0 ),
          m_column( 0 ),
          m_lexerCode( 0 )
        {
        }

        int m_errorCode;
        int m_line;
        int m_column;
        int m_lexerCode;
    };

    enum errorCodes
    {
        errNone,
        errExpectedActor,
        errExpectedActorName,
        errExpectedEOL,
        errExpectedReceives,
        errExpectedTupleType,
        errExpectedSourceActorName,
        errExpectedTupleAlias,
        errExpectedEnd,
        errExpectedStatement,
        errExpectedTupleTypeName,
        errExpectedValueName,
        errExpectedTupleOrTupleType,
        errExpectedTupleName,
        errExpectedTupleOrValue,
        errExpectedArrayElementName,
        errExpectedIn,
        errExpectedClosingParenthesis,
        errExpectedExpression,
        errExpectedThan,
        errDuplicateArgumentName,
        errExpectedArgumentName
    };

    static const char* s_errorMessages[];

    Parser( Deque< struct Lexer::Token >& lexerTokens,
            TupleRouter& tupleRouter,
            FunctionDispatcher& functionDispatcher,
            bool buildTree );

    bool parse( Linda2*& linda2,
                Vector< struct Highlighter::Token >* highlights = nullptr,
                Vector< Error >* errors = nullptr );

private:
    bool isNextCode( enum Lexer::Code code );
    struct Lexer::Token nextToken();
    void eatToken();
    void highlight( int attributes );
    void addError( int errorCode );

    bool actor( Actors::Linda2Actor*& _actor, bool firstActor );
    
    bool tupleHandler( TupleHandler*& _tupleHandler );
    
    bool block( Block*& _block );
    
    bool sendsStatement( Statements::Sends*& _sendsStatement );
    bool ifStatement( Statements::If*& _ifStatement );
    bool makesStatement( Statements::Makes*& _makesStatement );
    bool createsStatement( Statements::Creates*& _createsStatement );
    bool eachStatement( Statements::Each*& _eachStatement );
    bool whileStatement( Statements::While*& _whileStatement );

    bool expression( Expression*& value, int precedence );

    int nextTokenPrecedence();

    Deque< struct Lexer::Token >& m_lexerTokens;
    TupleRouter& m_tupleRouter;
    FunctionDispatcher& m_functionDispatcher;
    bool m_buildTree;
    
    Vector< struct Highlighter::Token >* m_highlights;
    Vector< Error >* m_errors;
};

} // namespace Linda2

} // namespace Agape

#endif // AGAPE_CARLO_PARSER_H
