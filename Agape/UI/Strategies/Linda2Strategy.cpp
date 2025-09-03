#include "Actors/Linda2Actor.h"
#include "InputDevices/InputDevice.h"
#include "Loggers/Logger.h"
#include "UI/Forms/Form.h"
#include "UI/Hotkeys.h"
#include "Utils/LiteStream.h"
#include "World/Compositor.h"
#include "World/Direction.h"
#include "ANSITerminal.h"
#include "Collections.h"
#include "ExecutionContext.h"
#include "FunctionDispatcher.h"
#include "Lexer.h"
#include "Linda2.h"
#include "Linda2Strategy.h"
#include "Parser.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Tuple.h"
#include "TupleHandler.h"
#include "TupleRouter.h"
#include "Value.h"
#include "WindowManager.h"

using Agape::String;

using namespace Agape::Carlo;
using namespace Agape::InputDevices;
using namespace Agape::Linda2;

namespace
{
    const String _Monitor( "Monitor" );
} // Anonymous namespace

namespace Agape
{

namespace UI
{

namespace Strategies
{

Linda2::Linda2( InputDevice& inputDevice,
                World::Compositor& compositor,
                WindowManager& windowManager,
                Hotkeys& hotkeys,
                TupleRouter& tupleRouter,
                FunctionDispatcher& functionDispatcher,
                const String& windowName ) :
  m_inputDevice( inputDevice ),
  m_compositor( compositor ),
  m_windowManager( windowManager ),
  m_hotkeys( hotkeys ),
  m_tupleRouter( tupleRouter ),
  m_functionDispatcher( functionDispatcher ),
  m_windowName( windowName ),
  m_completed( false ),
  m_active( false ),
  m_localOnly( true ),
  m_detail( false ),
  m_pause( false ),
  m_immediate( false ),
  m_terminal( nullptr ),
  m_currentForm( nullptr )
{
    WindowManager::TerminalWindow terminalWindow;
    if( m_windowManager.getTerminalWindow( m_windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }

    m_tupleRouter.registerMonitor( this );
}

Linda2::~Linda2()
{
    m_tupleRouter.deregisterMonitor( this );
    delete( m_currentForm );
}

void Linda2::enter( const Value& parameters )
{
    if( m_terminal )
    {
        m_windowManager.setTerminalWindowVisible( m_windowName, true );

        m_terminal->fillScreen( '\0', 0x00 );

        drawHotkeys();

        m_pause = false;
        m_active = true;

        m_completed = false;
    }
    else
    {
        m_completed = true;
    }
}

void Linda2::returnTo( const Value& parameters )
{
}

bool Linda2::calling( String& strategyName, Value& parameters )
{
    return false;
}

bool Linda2::returning( String& nextStrategy, Value& parameters )
{
    if( m_completed )
    {
        if( m_terminal )
        {
            m_windowManager.setTerminalWindowVisible( m_windowName, false );
        }
        m_hotkeys.clear();
        m_active = false;
        return true;
    }

    return false;
}

void Linda2::run()
{
    while( !m_inputDevice.eof() )
    {
        char c( m_inputDevice.get() );

        if( c == '\r' ) // Eat all CRs.
        {
            continue;
        }

        if( !m_immediate )
        {
            bool atEdge( false );
            if( c == Key::up )
            {
                m_compositor.walk( Direction::up, atEdge );
            }
            else if( c == Key::down )
            {
                m_compositor.walk( Direction::down, atEdge );
            }
            else if( c == Key::left )
            {
                m_compositor.walk( Direction::left, atEdge );
            }
            else if( c == Key::right )
            {
                m_compositor.walk( Direction::right, atEdge );
            }
            else if( c == control( 'a' ) )
            {
                m_compositor.actionMessage();
            }
            else if( c == 'i' )
            {
                m_immediate = true;
                drawImmediateForm();
                drawImmediateHotkeys();
            }
            else if( c == 'r' )
            {
                m_localOnly = !m_localOnly;
            }
            /*
            else if( c == 'd' )
            {
                m_detail = !m_detail;
            }
            */
            else if( c == 'p' )
            {
                m_pause = !m_pause;
                if( m_pause )
                {
                    m_terminal->consumeString( ANSITerminal::colour( Terminal::colYellow ) +
                                               "(Paused)\r\n" +
                                               ANSITerminal::reset() );
                }
                else
                {
                    m_terminal->consumeString( ANSITerminal::colour( Terminal::colYellow ) +
                                               "(Unpaused)\r\n" +
                                               ANSITerminal::reset() );
                }
            }
            else if( c == '\x1b' )
            {
                m_completed = true;
            }
        }
        else
        {
            if( c == '\x1b' )
            {
                closeForm();
                m_terminal->consumeString( ANSITerminal::colour( Terminal::colYellow ) +
                                           "(Prompt closed - Hit Esc again to close tuple view)\r\n" +
                                           ANSITerminal::reset() );
                m_immediate = false;
                drawHotkeys();
            }
            else if( c == '\n' )
            {
                runImmediate();
                closeForm();
                drawImmediateForm(); // To draw on line after previous result.
            }
            else if( c == Key::up )
            {
                recall();
            }
            else
            {
                if( m_currentForm )
                {
                    m_currentForm->consumeChar( c );
                }
            }
        }
    }

    m_compositor.run();
}

void Linda2::str( LiteStream& stream, int indent )
{
}

bool Linda2::accept( Tuple& tuple )
{
    if( !m_terminal ) return false;
    if( !m_active || m_pause || m_immediate ) return false;
    
    if( m_localOnly &&
        ( TupleRouter::sourceActor( tuple ) != _World ) &&
        ( TupleRouter::destinationActor( tuple ) != _World ) &&
        !( ( TupleRouter::sourceID( tuple ) == m_tupleRouter.myID() ) &&
           ( TupleRouter::destinationID( tuple ) == m_tupleRouter.myID() ) ) )
    {
        return false;
    }

    if( TupleRouter::tupleType( tuple ) == _Tick )
    {
        return false;
    }

    if( !m_detail )
    {
        m_terminal->consumeString( m_tupleRouter.transferDump( tuple ) + "\r\n" );
    }
    else
    {
        m_terminal->consumeString( tuple.dump() + "\r\n" );
    }

    return false; // Don't say we handled the tuple (we just snooped it).
}

bool Linda2::perform( Value& returnValue,
                      const String& name,
                      Map< String, Value* > arguments,
                      const String& caller )
{
    return false;
}

String Linda2::actorName() const
{
    return _Monitor;
}

void Linda2::drawHotkeys()
{
    m_hotkeys.clear();
    m_hotkeys.show( "\x18\x19\x1B\x1A", "Walk" );
    m_hotkeys.show( "C-A", "ction" );
    m_hotkeys.show( "I", "mmediate" );
    m_hotkeys.show( "R", "emote tps" );
    //m_hotkeys.show( "D", "etail" );
    m_hotkeys.show( "P", "ause" );
    m_hotkeys.show( "Esc", "Close" );
}

void Linda2::drawImmediateHotkeys()
{
    m_hotkeys.clear();
    m_hotkeys.show( "\x1B\x1A", "Move" );
    m_hotkeys.show( "\x18", "Last" );
    m_hotkeys.show( "Ret", "Execute" );
    m_hotkeys.show( "Esc", "Back" );
}

void Linda2::drawImmediateForm()
{
    if( !m_terminal ) return;

    m_terminal->consumeString( ANSITerminal::colour( Terminal::colWhite ) +
                               "Carlo-L2>" +
                               ANSITerminal::reset() );

    Vector< Forms::Field > fields;
    fields.push_back( Forms::Field( _command,
                                    m_terminal->width() - 11,
                                    1,
                                    m_terminal->row(),
                                    10,
                                    0x07 ) );

    closeForm();

    m_currentForm = new Forms::Form( m_windowManager,
                                     m_windowName,
                                     String(),
                                     fields,
                                     Forms::Title() );
    
    m_currentForm->openUI();

    if( m_currentForm->uiIsOpen() )
    {
        m_currentForm->draw();
    }
}

void Linda2::recall()
{
    if( !m_terminal || !m_currentForm ) return;

    m_currentForm->setFieldContents( _command, m_immediateString );
    m_currentForm->draw();
}

void Linda2::closeForm()
{
    if( m_currentForm != nullptr )
    {
        m_currentForm->close();
        delete( m_currentForm );
        m_currentForm = nullptr;
    }
}

void Linda2::runImmediate()
{
    if( !m_terminal || !m_currentForm ) return;

    m_immediateString = m_currentForm->getFieldContents( _command );

    Deque< Lexer::Token > tokens;
    Lexer lexer;

    // See if first token is an expression or statement.
    bool expression( false );
    lexer.lex( m_immediateString, tokens );
    if( !tokens.empty() )
    {
        const Lexer::Token& firstToken( tokens.front() );
        if( ( firstToken.m_code == Lexer::l_string ) ||
            ( firstToken.m_code == Lexer::l_float ) ||
            ( firstToken.m_code == Lexer::l_leftParen ) )
        {
            expression = true;
        }
    }
    tokens.clear();

    // Create dummy actor and tuple handler for the benefit of the parser.
    // If an expression, assign to dummy value.
    ExecutionContext executionContext;
    lexer.lex( "actor Immediate", tokens );
    lexer.lex( "receives Run", tokens );
    if( expression )
    {
        lexer.lex( "makes expr " + m_immediateString, tokens );
        executionContext.m_values["expr"] = new Value();
    }
    else
    {
        lexer.lex( m_immediateString, tokens );
    }
    lexer.lex( "end", tokens );
    lexer.lex( "end", tokens );

    Parser parser( tokens, m_tupleRouter, m_functionDispatcher, true );
    Carlo::Linda2* linda2( nullptr );
    Vector< Parser::Error > errors;
    if( parser.parse( linda2, nullptr, &errors ) )
    {
        Value value;
        if( linda2->evalOne( value, executionContext ) )
        {
            m_terminal->consumeString( ANSITerminal::colour( Terminal::colWhite ) +
                                       "\r\n=> " +
                                       ANSITerminal::reset() +
                                       value.dump() );
        }
        else
        {
            if( !executionContext.m_runtimeErrors.empty() )
            {
                ExecutionContext::RuntimeError& runtimeError( executionContext.m_runtimeErrors.front() );
                m_terminal->consumeString( ANSITerminal::colour( Terminal::colBrightRed ) +
                                           "\r\nError: " +
                                           String( ExecutionContext::s_errorMessages[runtimeError.m_code] ) +
                                           "\r\n" +
                                           ANSITerminal::reset() );
            }
        }
        delete( linda2 );
    }
    else
    {
        if( !errors.empty() )
        {
            Parser::Error parseError( errors.front() );
            m_terminal->consumeString( ANSITerminal::colour( Terminal::colBrightRed ) +
                                       "\r\nError: " +
                                       String( Parser::s_errorMessages[parseError.m_errorCode] ) +
                                       "\r\n" +
                                       ANSITerminal::reset() );
        }
    }
}

} // namespace Strategies

} // namespace UI

} // namespace Agape
