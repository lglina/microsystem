#ifndef AGAPE_COLLECTIONS_H
#define AGAPE_COLLECTIONS_H

#ifdef __WATCOMC__
#define throw
#define try if(true)
#define catch(...) if(false)
#endif

#include "Allocator.h"

#include <vector>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <functional>

#include "String.h"

#ifdef __WATCOMC__
#define Vector std::vector
#define Deque std::deque
#define List std::list
#define Map std::map
#define Set std::set
#else
namespace Agape
{

namespace Linda2
{
class Tuple;
}

template <class T>
using Vector = std::vector< T, Allocator< T > >;

template <class T>
using Deque = std::deque< T, Allocator< T > >;

template <class T>
using List = std::list< T, Allocator< T > >;

template <class T, class U>
using Map = std::map< T, U, std::less< T >, Allocator< std::pair< const T, U > > >;

template <class T>
using Set = std::set< T, std::less< T >, Allocator< T > >;

} // namesapce Agape

#endif

#endif // AGAPE_COLLECTIONS_H
