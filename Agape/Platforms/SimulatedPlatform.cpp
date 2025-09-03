#include "Timers/Factories/TimerFactory.h"
#include "Timers/Timer.h"
#include "Collections.h"
#include "KiamaFS.h"
#include "Platform.h"
#include "SimulatedPlatform.h"
#include "String.h"

#ifdef _WIN32
#include <heapapi.h>
#elif defined(__GNUC__)
#include <malloc.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace
{
    const int powerEventPeriod( 1000 ); // ms
} // Anonymous namespace

namespace Agape
{

namespace Platforms
{

Simulated::Simulated( Timers::Factory& timerFactory, KiamaFS* fs ) :
  m_fs( fs ),
  m_timer( timerFactory.makeTimer() )
{
}

Simulated::~Simulated()
{
    delete( m_timer );
}

void Simulated::performSelfTest()
{
}

void Simulated::doBootTasks()
{
    if( m_fs )
    {
        m_fs->purge();
        m_fs->createIndex();
    }
}

bool Simulated::error()
{
    // TEST
    //return true;

    return false;
}

void Simulated::currentErrors( Vector< enum ErrorType >& errors )
{
    // TEST
    //errors.push_back( displayError );
}

long Simulated::heapUsed()
{
#ifdef __EMSCRIPTEN__
    return 0;
#elif defined(_WIN32)
    // Not under WINE :(
    /*
    HEAP_SUMMARY heapSummary;
    memset( &heapSummary, 0, sizeof( heapSummary ) );
    heapSummary.cb = sizeof( heapSummary );
    HANDLE hDefaultProcessHeap( GetProcessHeap() );
    HeapSummary( hDefaultProcessHeap, 0, &heapSummary );
    return heapSummary.cbAllocated;
    */
    return 0;
#elif defined(__GNUC__)
    struct mallinfo2 mi( mallinfo2() );
    return mi.uordblks;
#else
    return 0;
#endif
}

struct Platform::PowerState Simulated::powerState()
{
    struct Platform::PowerState powerState;
    powerState.m_chargerState = Platform::battError;
    powerState.m_batteryPct = 80.0;
    powerState.m_runtimeHours = 0;
    powerState.m_runtimeMins = 0;
    return powerState;
}

void Simulated::notify( enum NotifyType type, enum NotifySource source )
{
#ifdef __EMSCRIPTEN__
    // Use only server-generated push notifications for Tela and ignore
    // all client-generated notifications, as client stops receiving chats
    // and checking for telegrams when tab becomes inactive.
    if( source == server )
    {
        switch( type )
        {
        case newChat:
            emscripten_run_script_string( "var a = document.getElementById(\"alert1\");\
                                        a.play();" );
            break;
        case newTelegram:
            emscripten_run_script_string( "var a = document.getElementById(\"alert2\");\
                                        a.play();" );
            break;
        default:
            break;
        }

        switch( type )
        {
        case newChat:
        case newTelegram:
            emscripten_run_script_string( "var i = document.getElementById(\"favicon1\");\
                                        i.href = i.getAttribute('data-href-alert');" );
            emscripten_run_script_string( "var i = document.getElementById(\"favicon2\");\
                                        i.href = i.getAttribute('data-href-alert');" );
            emscripten_run_script_string( "var i = document.getElementById(\"favicon3\");\
                                        i.href = i.getAttribute('data-href-alert');" );
            break;
        case connectionError:
            emscripten_run_script_string( "var i = document.getElementById(\"favicon1\");\
                                        i.href = i.getAttribute('data-href-offline');" );
            emscripten_run_script_string( "var i = document.getElementById(\"favicon2\");\
                                        i.href = i.getAttribute('data-href-offline');" );
            emscripten_run_script_string( "var i = document.getElementById(\"favicon3\");\
                                        i.href = i.getAttribute('data-href-offline');" );
            break;
        default:
            break;
        }
    }
#endif
}

void Simulated::cancelNotify( enum NotifyType type )
{
#ifdef __EMSCRIPTEN__
    emscripten_run_script_string( "var i = document.getElementById(\"favicon1\");\
                                   i.href = i.getAttribute('data-href');" );
    emscripten_run_script_string( "var i = document.getElementById(\"favicon2\");\
                                   i.href = i.getAttribute('data-href');" );
    emscripten_run_script_string( "var i = document.getElementById(\"favicon3\");\
                                   i.href = i.getAttribute('data-href');" );
#endif
}

void Simulated::run()
{
    /*
    if( m_timer->ms() >= powerEventPeriod )
    {
        Event event;
        event.m_type = Platform::powerStateChanged;
        dispatchEvent( event );
        m_timer->reset();
    }
    */
}

} // namespace Platforms

} // namespace Agape
