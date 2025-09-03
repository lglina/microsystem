#include "Clocks/Clock.h"
#include "Utils/LiteStream.h"
#include "FunctionDispatcher.h"
#include "String.h"
#include "StringConstants.h"
#include "Terminal.h"
#include "Tuple.h"
#include "TupleRouter.h"
#include "TupleRoutingCriteria.h"
#include "VRTime.h"
#include "WindowManager.h"

using namespace Agape::Linda2;

namespace Agape
{

namespace UI
{

VRTime::VRTime( WindowManager& windowManager,
                const String& windowName,
                TupleRouter& tupleRouter,
                FunctionDispatcher& functionDispatcher ) :
  Native( "MyClock" ),
  m_tupleRouter( tupleRouter ),
  m_functionDispatcher( functionDispatcher ),
  m_terminal( nullptr ),
  m_registered( false ),
  m_haveTime( false ),
  m_running( false ),
  m_secsSinceEpoch( 0 )
{
    WindowManager::TerminalWindow terminalWindow;
    if( windowManager.getTerminalWindow( windowName, terminalWindow ) )
    {
        m_terminal = terminalWindow.m_terminal;
    }

    m_tupleRouter.registerActor( this );
    m_functionDispatcher.registerActor( this );
}

VRTime::~VRTime()
{
    // We could remove the routing criteria here, but if we're destructing
    // this then we're also disconneting/disconnected from the server anyway.
    m_tupleRouter.deregisterActor( this );
    m_functionDispatcher.deregisterActor( this );
}

void VRTime::doRegister()
{
    TupleRoutingCriteria tupleRoutingCriteria;
    tupleRoutingCriteria.m_types.push_back( new Value( _Time ) );
    m_tupleRouter.sendAddRoutingCriteriaRequest( tupleRoutingCriteria );

    m_registered = true;
}

void VRTime::draw()
{
    m_running = true;

    int day = 0, month = 0, year = 0;
    int hour = 0, minute = 0, second = 0;
    int dayOfWeek = 0;
    Clock::timestampToParts( m_secsSinceEpoch,
                             day,
                             month,
                             year,
                             hour,
                             minute,
                             second,
                             dayOfWeek );
    Clock::utcToVRT( day,
                     month,
                     year,
                     hour,
                     minute,
                     second,
                     dayOfWeek );
    String dateTimeStr( Clock::dateTimeToString( day,
                                                 month,
                                                 year,
                                                 hour,
                                                 minute,
                                                 second,
                                                 dayOfWeek ) );

    m_terminal->consumeNext( 0, m_terminal->width() - dateTimeStr.length() );
    m_terminal->consumeString( dateTimeStr, Terminal::scrollLock );
}

void VRTime::start()
{
    m_running = true;
}

void VRTime::stop()
{
    if( !m_terminal ) return;

    // Redundant if we're sharing the same terminal as for Navigation, as it
    // will clear the entire terminal as well.
    m_terminal->clearScreen();

    m_running = false;
}

bool VRTime::haveTime()
{
    return m_haveTime;
}

void VRTime::waitForTime()
{
    while( m_registered && !m_haveTime )
    {
        m_tupleRouter.run(); // Run to receive time tuple. FIXME: Timeout?
    }
}

bool VRTime::accept( Tuple& tuple )
{
    if( TupleRouter::tupleType( tuple ) == _Time )
    {
        if( tuple.hasValue( _now ) )
        {
            m_secsSinceEpoch = tuple[_now];

            // FIXME: Do we want the drawing to be driven by events here,
            // or should WalkStrategy be calling draw()?
            if( m_running )
            {
                draw();
            }
            // If not m_running we don't draw but still process incoming time
            // tuples, for the benefit of Clocks::VRTime.

            m_haveTime = true;
        }
    }

    return false; // Don't report handled so tuple can be passed on?
}

bool VRTime::perform( Value& returnValue,
                      const String& name,
                      Map< String, Value* > arguments,
                      const String& caller )
{
    if( name == _now )
    {
        waitForTime();
        returnValue = (double)m_secsSinceEpoch;
        return true;
    }

    return false;
}

} // namespace UI

} // namespace Agape
