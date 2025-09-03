#include "Allocator.h"

#if defined(__PIC32MX__)
#include "GraphicsDrivers/GraphicsDriver.h"
#include "InterruptHandler.h"
#endif

int g_maxAlloc( 0 );
void* g_maxAllocAddress( 0 );

void* operator new( size_t size )
{
    unsigned int callerAddress( 0 );
#if defined(__PIC32MX__)
    asm volatile("move %0,$ra" : "=r" (callerAddress));
#endif
    char* p = (char*)malloc( size );
    if( p == nullptr )
    {
#if defined(__PIC32MX__)
        Agape::panic( callerAddress, EXCEPTION_CODE_NEW );
#endif
        while( 1 )
		{
			asm( "nop" );
		}
    }
    if( size > g_maxAlloc ) g_maxAlloc = size;
    if( ( p + size ) > g_maxAllocAddress ) g_maxAllocAddress = p + size;

    return p;
}

void operator delete( void* p ) noexcept
{
    free( p );
}

namespace Agape
{

#if defined(__PIC32MX__)
void panic( unsigned int callerAddress, unsigned int code )
{
    if( Agape::InterruptDispatcher::s_graphicsDriver )
    {
        Agape::InterruptDispatcher::s_graphicsDriver->panic( callerAddress, code );
    }
}
#endif

} // namespace Agape
