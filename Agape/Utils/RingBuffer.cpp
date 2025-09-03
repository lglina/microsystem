#include "RingBuffer.h"

namespace Agape
{

template< class T >
RingBuffer< T >::RingBuffer( int capacity ) :
  m_capacity( capacity ),
  m_pushOffset( 0 ),
  m_popOffset( 0 ),
  m_values( new T[ capacity ] )
{
}

template< class T >
RingBuffer< T >::~RingBuffer()
{
    delete[]( m_values );
}

template< class T >
void RingBuffer< T >::push( const T& val )
{
    m_values[ m_pushOffset ] = val;
    int nextPushOffset( m_pushOffset + 1 );
    if( nextPushOffset == m_capacity )
    {
        nextPushOffset = 0;
    }
    m_pushOffset = nextPushOffset;
}

template< class T >
T RingBuffer< T >::pop()
{
    T value( m_values[ m_popOffset ] );
    int nextPopOffset( m_popOffset + 1 );
    if( nextPopOffset == m_capacity )
    {
        nextPopOffset = 0;
    }
    m_popOffset = nextPopOffset;
    return value;
}

template< class T >
T RingBuffer< T >::front()
{
    T value( m_values[ m_popOffset ] );
    return value;
}

template< class T >
void RingBuffer< T >::clear()
{
    m_popOffset = m_pushOffset;
}

template< class T >
bool RingBuffer< T >::isFull() const
{
    int nextOffset = m_pushOffset + 1;
    if( nextOffset == m_capacity )
    {
        nextOffset = 0;
    }
    return( nextOffset == m_popOffset );
}

template< class T >
bool RingBuffer< T >::isEmpty() const
{
    return( m_pushOffset == m_popOffset );
}

template< class T >
int RingBuffer< T >::free() const
{
    int pushOffset( m_pushOffset );
    int popOffset( m_popOffset );

    // | 0 | 1 | 2 | 3 | 4 |
    // |psh|   |gap|pop|   | = 2

    // | 0 | 1 | 2 | 3 | 4 |
    // |gap|pop|   |psh|   | = 2

    int free;
    if( pushOffset < popOffset )
    {
        free = popOffset - pushOffset - 1;
    }
    else
    {
        // Front free + Back free - gap.
        free = popOffset + ( m_capacity - pushOffset ) - 1;
    }

    return free;
}

template< class T >
int RingBuffer< T >::size() const
{
    return m_capacity - free() - 1;
}

template class RingBuffer< char >;

} // namespace Agape
