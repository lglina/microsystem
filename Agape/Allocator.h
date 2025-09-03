#ifndef AGAPE_ALLOCATOR_H
#define AGAPE_ALLOCATOR_H

#if defined(__PIC32MX__)
#include "InterruptHandler.h"
#endif

#include <cstddef>
#include <stdlib.h>

#ifdef __WATCOMC__
#define noexcept
#endif

using namespace std;

extern int g_maxAlloc;
extern void* g_maxAllocAddress;

// Override global new() and delete() to monitor heap allocations.
void* operator new( size_t size ) __attribute__((malloc));
void operator delete( void* p ) noexcept;

namespace Agape
{

// Handler to draw "sad Agape" with caller address and error code.
#if defined(__PIC32MX__)
void panic( unsigned int callerAddress, unsigned int code );
#endif

// Much thanks to JeanHeyd Meneide for this troll allocator.
// https://thephd.dev/freestanding-noexcept-allocators-vector-memory-hole
// Allows us to use STL containers with -fno-exceptions.
template <class T>
class Allocator
{
public:
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef T*        pointer;
	typedef const T*  const_pointer;
	typedef T&        reference;
	typedef const T&  const_reference;
	typedef T         value_type;

	Allocator() {}
	Allocator( const Allocator& ) {}

	pointer allocate( std::size_t element_count ) noexcept;
	void deallocate( T* first, std::size_t ) noexcept;

	pointer           address( reference x ) const { return &x; }
	const_pointer     address( const_reference x ) const { return &x; }
	Allocator<T>&     operator=( const Allocator& ) { return *this; }
	void              construct( pointer p, const T& val ) 
					  { /*__builtin_disable_interrupts();*/ new ( ( T* ) p ) T( val ); /*__builtin_enable_interrupts();*/ }
	void              destroy( pointer p ) { p->~T(); }

	size_type         max_size() const { return size_t( -1 ); }

	void validate_max( std::size_t element_count, std::size_t container_maximum ) noexcept;

	template <class U>
	struct rebind { typedef Allocator<U> other; };

	template <class U>
	Allocator( const Allocator<U>& ) {}

	bool operator==( const Allocator& ) { return true; }
	bool operator!=( const Allocator& ) { return false; }
};

template <typename T>
T* Allocator<T>::allocate( std::size_t element_count ) noexcept
{
	unsigned int callerAddress( 0 );
#if defined(__PIC32MX__)
    asm volatile("move %0,$ra" : "=r" (callerAddress));
#endif
	// No alignment.
	std::size_t byte_count = element_count * sizeof( T );

	void* ptr = malloc( byte_count );
	if( ptr == nullptr )
	{
#if defined(__PIC32MX__)
		panic( callerAddress, EXCEPTION_CODE_ALLOCATOR );
#endif
		while( 1 )
		{
			asm( "nop" );
		}
	}

	return static_cast<T*>( ptr );
}

template <typename T>
void Allocator<T>::deallocate( T* first, std::size_t ) noexcept
{
	free( (void*)first );
}

template <typename T>
void Allocator<T>::validate_max( std::size_t element_count, std::size_t container_max ) noexcept
{
	// Don't validate! Assume caller knows what they're doing.
}

} // namespace Agape

#endif // AGAPE_ALLOCATOR_H
