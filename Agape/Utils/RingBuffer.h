#ifndef AGAPE_UTILS_RING_BUFFER_H
#define AGAPE_UTILS_RING_BUFFER_H

namespace Agape
{

template< class T >
class RingBuffer
{
public:
    RingBuffer( int capacity );
    ~RingBuffer();

    void push( const T& val );
    T pop();
    T front();
    void clear();

    bool isFull() const;
    bool isEmpty() const;

    // N.B.: Depending on wherther these are called by the producer
    // or consumer, they may report a +/- one item if the consumer
    // or producer (respectively) pops or push'es in an interrupt.
    int free() const;
    int size() const;

private:
    const int m_capacity;
    volatile int m_pushOffset;
    volatile int m_popOffset;

    volatile T* m_values;
};

} // namespace Agape

#endif // AGAPE_UTILS_RING_BUFFER_H
