#ifndef AGAPE_ARRAY_DELETER_H
#define AGAPE_ARRAY_DELETER_H

namespace Agape
{

template< typename T >
struct ArrayDeleter
{
    void operator()( T* const p )
    {
        delete[] p;
    }
};

} // namespace Agape

#endif // AGAPE_ARRAY_DELETER_H
